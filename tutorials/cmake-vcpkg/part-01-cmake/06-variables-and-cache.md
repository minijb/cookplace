---
title: 变量与缓存：configure 期的状态
series: cmake-vcpkg
chapter: part-01/06
topic: 区分普通变量与缓存变量，理解 CMAKE_BUILD_TYPE、option 与生成器表达式出现的时机
audience: 前端游戏开发工程师
outcome: 知道每类变量活到什么时候、在哪查改，Debug/Release 在两种生成器下分别怎么控制
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

用 `-D` 与缓存正确互动，理解单/多配置差异，读懂生成器表达式出现的时机。

## -D 与 option：configure 的参数面

```powershell
# 命令行设置缓存变量（等价于永久改配置）
cmake -B build -DCMAKE_BUILD_TYPE=Release -DMYPROJ_BUILD_TOOLS=ON
```

`option(MYPROG_BUILD_TOOLS "Build tools" OFF)` 声明的就是布尔缓存变量，与 `-D` 打通的。**改了缓存变量必须重新 configure**（VSCode 里点底部的 Build 即可）。

## Debug / Release：单配置 vs 多配置

- **Ninja（单配置）**：`CMAKE_BUILD_TYPE` 在 configure 期定死，产物在 `build/` 根。要 Debug 和 Release 两份就配两个构建目录（presets 自动帮你分，见 08 课）
- **VS 生成器（多配置）**：configure 不定型，build 时 `cmake --build build --config Release`，产物在 `build/Release/`

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
