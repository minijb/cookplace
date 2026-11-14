---
title: CMake 核心概念
brief: CMake 的三阶段模型、目标与属性传播、依赖三方案、预设与生成器等核心知识
tags: [cmake, build-system]
---
# CMake 核心概念

## 三阶段模型
CMake 是「构建系统生成器」（meta build system）：`CMakeLists.txt` 不是被直接构建，而是被翻译成 Ninja/Makefile/MSBuild 等原生构建脚本。
- 配置期（configure）：执行 CMake 语言，生成器表达式不求值，产物缓存在 `CMakeCache.txt`
- 生成期（generate）：解析依赖图、求值生成器表达式，产出原生构建文件（out-of-source 构建目录）
- 构建期（build）：由原生工具执行编译链接，CMake 只做转发（`cmake --build`）

## 目标与属性传播
- target 是核心抽象：executable / library(STATIC|SHARED|INTERFACE|OBJECT) / alias
- 用法要求（include 路径、编译定义、链接库）挂在 target 上，而不是全局 include_directories/link_libraries
- 传播关键字：`PRIVATE`（仅自身消费）、`INTERFACE`（仅下游消费）、`PUBLIC`（自身+下游）；对头文件里的依赖必须 INTERFACE/PUBLIC，纯 .cpp 里 PRIVATE
- generator expressions（`$<...>`）在生成期求值，用于按配置/平台/目标条件化（如 `$<$<CONFIG:Debug>:...>`）

## 依赖三方案与取舍
| 方案 | 适用 | 要点 |
|---|---|---|
| find_package | 系统/预装库 | 模块模式（Find*.cmake）vs 配置模式（*Config.cake，更可靠）； Imported target 优先于裸变量 |
| add_subdirectory | 源码在手的项目内模块 | 同一次 CMake 运行，全工程统一编译选项 |
| FetchContent | 源码不在手的第三方库 | 配置期拉取源码再 add_subdirectory；配合 GIT_TAG 固定版本 |

CPM.cmake 是 FetchContent 的封装（自动缓存、声明的包不重复），生态内常见。

## 预设与工程化
- CMakePresets.json / CMakeUserPresets.json：把常用 `-G/-D/-B` 组合成具名 preset（configure/build/test 三类，可继承），命令行退化为 `cmake --preset xxx`；跨平台免写脚本
- 多配置生成器（VS/Xcode）vs 单配置（Ninja/Makefiles）：`CMAKE_BUILD_TYPE` 仅单配置有效；配置无关写法用生成器表达式
- 版本策略：`cmake_minimum_required` 必写（同时影响策略行为）；现代写法全用 target 命令，避免老式目录级命令

## 常用命令速记
`cmake --preset <p>` → `cmake --build --preset <p>` → `ctest --preset <p>`；`cmake -S . -B build` 显式指定源/构建目录；`cmake --build build --target <t>` 构建单目标。

## 待深入清单
- [ ] find_package 配置模式细节：包配置文件的查找顺序与包版本文件
- [ ] INTERFACE 目标与 header-only 库的惯用法
- [ ] 交叉编译工具链文件（CMAKE_TOOLCHAIN_FILE）→ 见 `integration.md`
- [ ] CMake 语言能力边界：function/macro、作用域（目录/函数作用域与 PARENT_SCOPE）
- [ ] install/export：库的安装与 `find_package` 化（export set）
- [ ] 与 IDE（VSCode CMake Tools / CLion / VS）的集成行为
