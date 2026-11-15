---
title: 变量与缓存：configure 期的状态
series: cmake-vcpkg
chapter: part-01/06
topic: 区分普通变量与缓存变量，理解 CMAKE_BUILD_TYPE、option 与生成器表达式出现的时机
audience: 前端游戏开发工程师
outcome: 知道每类变量活到什么时候、在哪查改，Debug/Release 在两种生成器下分别怎么控制、两种配置到底差在哪
environment: Windows 11；CMake 3.29.2
date: 2026-11-14
---

# 变量与缓存：configure 期的状态

CMake 语言的变量分三种，命和作用域各不相同：

| 种类 | 声明方式 | 存活范围 | 修改入口 |
|---|---|---|---|
| 普通变量 | `set(x 1)` | 当前目录/函数作用域，子作用域可见副本 | CMakeLists.txt |
| 缓存变量 | `set(x 1 CACHE STRING "说明")` | 跨 configure 持久化在 `CMakeCache.txt` | 命令行 `-D`、`cmake-gui`、CMake Tools |
| 环境变量 | `$ENV{PATH}` | 进程环境 | 系统设置 |

**优先级**：普通变量遮蔽同名缓存变量。找不到「为什么我改了没生效」时，八成是作用域里有个同名普通变量，或值已进缓存——直接看 `build/CMakeCache.txt` 搜变量名。

## 你将学会什么

用 `-D` 与缓存正确互动，理解单/多配置差异与 Debug/Release 的 flags 区别，读懂生成器表达式出现的时机。

## -D 与 option：configure 的参数面

```powershell
# 命令行设置缓存变量（等价于永久改配置）
cmake -B build -DCMAKE_BUILD_TYPE=Release -DMYPROJ_BUILD_TOOLS=ON
```

`option(MYPROG_BUILD_TOOLS "Build tools" OFF)` 声明的就是布尔缓存变量，与 `-D` 打通的。**改了缓存变量必须重新 configure**（VSCode 里点底部的 Build 即可）。

## Debug / Release：单配置 vs 多配置

- **Ninja（单配置）**：`CMAKE_BUILD_TYPE` 在 configure 期定死，产物在 `build/` 根。要 Debug 和 Release 两份就配两个构建目录（presets 自动帮你分，见 08 课）
- **VS 生成器（多配置）**：configure 不定型，build 时 `cmake --build build --config Release`，产物在 `build/Release/`

### 型里装了什么：两捆预打包 flags

configuration 不是模式开关，而是 CMake 预打包的编译/链接 flags 捆（MSVC 默认值）：

| 维度 | Debug | Release | 实际后果 |
|---|---|---|---|
| 优化 | `/Od` 禁优化 + `/Ob0` 禁内联 | `/O2` 速度最大化 + `/Ob2` 自动内联 | Debug 慢 10~100 倍，但调试器里变量真实可看、断点行号准确；Release 里变量常被「优化没了」 |
| 调试信息 | `/Zi` 完整 PDB | 无 | Debug 能单步/看栈，Release 不能 |
| 运行时库（CRT） | `/MDd` debug CRT | `/MD` release CRT | 两边必须一致，混链报 `LNK2038`（见下） |
| 预处理器宏 | 定义 `_DEBUG` | 定义 `NDEBUG` | `NDEBUG` 关掉所有 `assert()`——「只在 Release 出现的 bug」常源于此（UB/未初始化被优化暴露） |
| 运行时检查 | `/RTC1` 栈/未初始化检查 | 无 | Debug 下越界、未初始化当场被抓，Release 下静默带病运行 |

GCC/Clang 对应：Debug = `-g`，Release = `-O3 -DNDEBUG`（无 CRT 之分，Linux 上少一类坑）。类比前端：Debug ≈ `vite dev`（慢、可调试、全量检查），Release ≈ `vite build --mode production`（压缩、DevTools 断言消失）。

CRT 那行是 Windows + vcpkg 环境的关键坑：vcpkg 每个包都装两份（`vcpkg_installed/x64-windows/debug/` 与 release 根目录），CMake 按 configuration 自动选对应那份——多配置生成器把 Debug/Release 产物放在 `build/Debug/`、`build/Release/` 物理隔离，防的就是 debug CRT 的库链进 release CRT 的程序。

其实有四捆，按需选用：

| configuration | 定位 | 游戏开发典型用途 |
|---|---|---|
| `Debug` | 无优化 + 调试信息 | 日常开发、抓崩溃 |
| `RelWithDebInfo` | `/O2` + 调试信息 | 性能分析：要 Release 的真实帧率，又要在崩溃栈里看到行号 |
| `Release` | `/O2`，无调试信息 | 出包 |
| `MinSizeRel` | `/O1` 体积最小 | 安装包体积敏感时 |

> ⚠️ Ninja 下忘记设 `CMAKE_BUILD_TYPE` 时它是**空字符串**——既不是 Debug 也不是 Release，无优化也无调试信息，还容易撞上「vcpkg 的 debug 库没链接」问题。preset 里显式写明（08 课）。

## 生成器表达式：配置期语言里的「生成期占位」

`$<...>` 在 generate 阶段求值，用于「按配置/平台/编译器」条件化，且**能出现在 target 属性里**（`if()` 不能）：

```cmake
# 只有 MSVC 加 /utf-8；只有 Debug 定义 _DEBUG_FEATURE
target_compile_options(app PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
    $<$<CONFIG:Debug>:-D_DEBUG_FEATURE>
)
```

part-03 的 capstone 用了等价的 `if(MSVC)` 写法——两种都对，`if` 直观、genex 能下钻到属性与链接的传递。

## 作用域一句话

普通变量沿目录树向下继承（子目录拿到副本，改了不回传；`PARENT_SCOPE` 显式回传）。**能用 target 属性表达的，一律不用变量接力**——变量接力是老 CMake 的腐化源头。

## 下一步

[07 · 依赖三方案](07-dependencies.md)——库从哪来。
