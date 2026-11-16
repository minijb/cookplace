---
title: 第四章：CMake 工程、依赖与目录结构
series: opengl-compute-shader
chapter: 第四章（第 0 部第 4 课）
topic: 用 CMakePresets 和 vcpkg Manifest 构建可重复配置的 C++17 OpenGL 工程
audience: 已运行第三章窗口示例，希望理解每个构建文件与依赖职责的初学者
outcome: 能解释 CMakeLists、vcpkg.json、CMakePresets 和源码/构建目录的职责，并独立配置、构建、测试和清理工程
environment: C++17；CMake 3.21+；Ninja；vcpkg Manifest Mode；GLFW 3 + GLAD 1；Windows 11 + MSVC 验证
date: 2026-11-16
---
# 第四章：CMake 工程、依赖与目录结构

适用环境：C++17、CMake 3.21 或更高版本、Ninja、vcpkg Manifest Mode、GLFW 3 和 GLAD 1。示例以跨平台 CMake 描述工程，但本章只声明已经实际验证的平台，不把所有系统视为已测试。

你将学会：

- 说明 CMake、编译器、Ninja 和 vcpkg 各自的职责。
- 理解 `CMakeLists.txt` 中的语言标准、依赖查找、Target 和链接。
- 使用 `vcpkg.json` 声明依赖，用 `CMakePresets.json` 固定配置。
- 区分源码目录、构建目录和依赖安装目录。
- 从干净构建目录重新配置、编译并运行 CTest。

完整示例位于 [code/part-00-04-project](../code/part-00-04-project/)。程序行为与第三章相近，本章重点是让它成为结构清楚、可以重复构建的工程。

## 为什么把工程配置单独讲清楚

“本机能运行”可能依赖许多没有被记录的条件：编译器环境、库的安装位置、构建类型或手工复制过的 DLL。只要换目录或换电脑，程序就可能无法配置。

本章的目标是把隐含条件写成文件：

- 需要哪些依赖，写进 `vcpkg.json`。
- 怎样生成和编译 Target，写进 `CMakeLists.txt`。
- 使用哪个生成器、构建目录和 Toolchain，写进 `CMakePresets.json`。
- 哪些产物不应提交，写进 `.gitignore`。

这样，学习下一章时就不必重新猜测工程如何搭建。

## 工具职责不要混淆

| 工具 | 职责 |
|---|---|
| CMake | 读取工程描述，生成具体构建系统需要的文件 |
| Ninja | 根据 CMake 生成的规则执行编译和链接 |
| MSVC、GCC 或 Clang | 编译 C++ 源码并链接目标 |
| vcpkg | 按 Manifest 获取和构建第三方依赖，并向 CMake 提供包配置 |
| CTest | 注册并运行测试，不替代编译器或测试程序本身 |

CMake 本身通常不直接编译你的 `.cpp` 文件；它负责组织构建规则。Ninja 是这些规则的一种执行工具。

## 示例目录结构

```text
part-00-04-project/
├── CMakeLists.txt
├── CMakePresets.json
├── vcpkg.json
└── src/
    └── main.cpp
```

四个输入文件都属于源码工程：

- `src/main.cpp`：程序入口。
- `CMakeLists.txt`：工程与 Target 描述。
- `vcpkg.json`：第三方依赖和版本基线。
- `CMakePresets.json`：可重复的配置、构建和测试入口。

运行配置后生成的 `build/` 不属于手写源码。示例上层的 [code/.gitignore](../code/.gitignore) 已忽略 `build/`、`out/` 和 `vcpkg_installed/`。

## 从 CMakeLists 读懂工程

### 最低版本与项目语言

```cmake
cmake_minimum_required(VERSION 3.21)
project(opengl_part00_ch04 LANGUAGES CXX)
```

最低版本 3.21 与本例使用的 Preset 格式相匹配。`LANGUAGES CXX` 声明这是 C++ 项目，让 CMake 检测相应编译器。

### C++17 设置

```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
```

这三行分别表示：使用 C++17、要求编译器支持该标准、关闭编译器专有语言扩展。关闭扩展不是禁止 OpenGL 扩展，两者完全无关。

### 查找依赖

```cmake
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
```

- `OpenGL::GL` 来自 CMake 的系统 OpenGL 查找模块。
- `glfw3` 和 `glad` 由 vcpkg 提供 Config Package。
- `REQUIRED` 表示找不到就停止配置，避免把错误推迟到更难理解的编译或链接阶段。

### 创建 Target 并链接

```cmake
add_executable(ch04_project src/main.cpp)
target_compile_definitions(ch04_project PRIVATE GLFW_INCLUDE_NONE)
target_link_libraries(ch04_project PRIVATE glfw glad::glad OpenGL::GL)
```

`add_executable` 创建名为 `ch04_project` 的可执行 Target。后续设置直接挂到 Target，而不是无差别修改整个目录。

`PRIVATE` 表示这些设置只用于当前 Target；本例没有被其他 Target 依赖的库接口，因此不需要暴露为 `PUBLIC`。

`GLFW_INCLUDE_NONE` 防止 GLFW 自动包含 OpenGL 头文件，让 GLAD 统一提供 OpenGL 声明。

### MSVC UTF-8 与 CTest

```cmake
if(MSVC)
    target_compile_options(ch04_project PRIVATE /utf-8)
endif()
```

这只在 MSVC 下添加 `/utf-8`，让包含中文注释的 UTF-8 源码按一致编码处理。其他编译器不接收这个 MSVC 参数。

`include(CTest)` 和 `add_test` 注册一个隐藏窗口、两帧退出的冒烟测试。测试不是证明渲染算法正确，而是验证工程能够启动、创建 Context 并完成主循环。

## vcpkg.json：声明依赖与基线

本例 Manifest 包含：

- `glfw3`：窗口、Context 和输入。
- `glad`，并启用 `gl-api-43` 与 `loader` Feature：提供最高到 OpenGL 4.3 的声明和加载支持。当前程序仍只请求 3.3 Core。
- `builtin-baseline`：固定 vcpkg Registry 的依赖解析基线。

`gl-api-43` 并不意味着本章必须创建 4.3 Context；它只为第七章 Debug Output 和后续课程保留相同依赖配置。

使用 Manifest Mode 时，不需要在源码目录手工执行一次全局 `vcpkg install` 才能表达项目依赖。CMake 通过 vcpkg Toolchain 配置项目时，会根据 Manifest 安装所需包到构建树中。

## CMakePresets.json：把命令行保存下来

本例 `debug` Preset 固定了：

- `generator` 为 `Ninja`。
- `binaryDir` 为 `${sourceDir}/build/debug`。
- `CMAKE_BUILD_TYPE` 为 `Debug`。
- `CMAKE_TOOLCHAIN_FILE` 为 `$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake`。
- `CMAKE_EXPORT_COMPILE_COMMANDS` 为 `ON`，供 clangd 等工具读取。

`$env{VCPKG_ROOT}` 表示从环境变量读取 vcpkg 根目录。开始配置前必须设置它，且路径中应存在 `scripts/buildsystems/vcpkg.cmake`。

同名 `buildPresets` 和 `testPresets` 分别引用该配置，因此配置、构建和测试不需要重复输入一长串参数。

> ⚠️ Ninja 是单配置生成器，`Debug` 与 `Release` 通常应使用不同构建目录。本例只提供 `debug`，后续需要 Release 时再添加独立 Preset 和目录。

## 配置、构建和测试

先准备：

1. 安装 CMake 3.21+、Ninja 和 C++17 编译器。
2. 安装 vcpkg，并设置 `VCPKG_ROOT`。
3. 在 Windows + MSVC 下，使用已初始化 MSVC 环境的终端；确保 `cl` 和 Ninja 可用。

进入 `code/part-00-04-project` 后执行：

```powershell
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

三个阶段各自回答一个问题：

1. **Configure**：编译器、Toolchain 和依赖能否正确解析？
2. **Build**：源码能否编译并链接为可执行文件？
3. **Test**：生成的程序能否完成指定冒烟测试？

Windows 下运行：

```powershell
.\build\debug\ch04_project.exe
```

预期看到窗口，并输出 `Project configured with CMake + vcpkg` 和实际 OpenGL 版本。

## 构建产物放在哪里

本例使用：

```text
part-00-04-project/
├── src/
├── build/
│   └── debug/
│       ├── ch04_project.exe
│       ├── compile_commands.json
│       └── vcpkg_installed/
└── ...
```

具体文件会因平台不同而变化。重点是：

- 不手工编辑 `build/` 中的生成文件来修复工程；应修改 `CMakeLists.txt` 或 Preset。
- Manifest 依赖安装到构建树后，不需要复制到源码根目录。
- 删除 `build/debug` 后重新执行三条命令，就是一次干净重建。

## 常见错误

| 错误信息或现象 | 原因与处理 |
|---|---|
| `CMAKE_TOOLCHAIN_FILE` 路径无效 | 检查 `VCPKG_ROOT` 是否设置，以及 Toolchain 文件是否存在 |
| 找不到 `glfw3Config.cmake` 或 GLAD 包 | 确认 vcpkg Toolchain 在首次配置时生效，并查看 Manifest 安装日志 |
| 找不到编译器 | 安装编译器；MSVC 下使用正确开发者环境；不要只安装 CMake |
| Visual Studio STL 报告 `STL1000` | 当前 Clang 与 Visual Studio STL 版本不兼容；改用受支持的 Clang 或 MSVC，不要关闭 STL 检查 |
| 修改 Preset 后仍使用旧配置 | 删除对应构建目录后重新配置，避免旧 Cache 干扰 |
| 源码目录出现手工 `vcpkg_installed` | 区分手工经典模式安装和本例构建树中的 Manifest 安装 |
| 可执行文件运行时找不到 DLL | 使用本例完整构建输出目录运行，检查 vcpkg 是否完成 app-local DLL 复制 |

## 练习

1. 删除 `build/debug`，重新执行配置、构建和测试，记录三个阶段分别生成了什么。
2. 把可执行 Target 名改为 `opengl_orientation`，同步修改源码中的用法提示和 CTest 命令。
3. 为工程添加一个独立的 `Release` Preset，使用 `build/release`，并比较两种构建类型的 Cache。
4. 临时移除 `GLFW_INCLUDE_NONE`，阅读相关头文件说明；不要为了“试试看”再引入另一套 OpenGL 头文件。

下一章暂时离开构建工具，使用 GLM 学习向量、矩阵、点积、叉积和齐次坐标。

## 参考资料

- [CMake Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/)：项目、Target 和构建基础。
- [CMake `cmake-presets` 手册](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)：Preset 文件与环境变量展开。
- [CMake `find_package` 手册](https://cmake.org/cmake/help/latest/command/find_package.html)：依赖查找与 Config Mode。
- [vcpkg Manifest Mode](https://learn.microsoft.com/vcpkg/concepts/manifest-mode)：Manifest、依赖和构建树安装。
- [vcpkg 与 CMake 集成](https://learn.microsoft.com/vcpkg/users/buildsystems/cmake-integration)：Toolchain 和 `find_package` 集成。
