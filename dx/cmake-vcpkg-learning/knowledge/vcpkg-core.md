---
title: vcpkg 核心概念
brief: vcpkg 的 port/ports tree、classic 与 manifest 两种模式、triplets、版本约束、双重缓存与 registries
tags: [vcpkg, package-manager, cpp]
---
# vcpkg 核心概念

## 定位与 port 模型
微软开源的 C++ 依赖管理器（C++ 跨平台包管理的事实标准之一）。核心抽象是 port：一个描述「如何从源码构建某库」的配方（portfile.cmake + vcpkg.json），全部官方配方构成 ports tree。vcpkg 的哲学是**从源码构建**（按 triplet 编译进本机），而非下载预编译二进制——因此首个依赖安装较慢，但与构建环境天然一致。

## 两种使用模式
| | classic 模式 | manifest 模式（推荐） |
|---|---|---|
| 声明位置 | 全局命令行 `vcpkg install <pkg>` | 项目根 `vcpkg.json`（≈ package.json） |
| 安装位置 | vcpkg 安装目录内 | 项目内 `vcpkg_installed/`（≈ node_modules，gitignore） |
| 依赖可见性 | 全局共享、易漂移 | 按项目隔离、可提交锁定 |
| 适用场景 | 临时实验 | 一切正式项目 |

manifest 文件要素：`dependencies`（可带版本约束）、`builtin-baseline`（锁定 ports tree 版本）、`features`（可选特性组）、`overrides`（强制版本）。

## triplets
triplet = 目标平台三元组（架构-OS-链接库类型），如 `x64-windows`（动态）、`x64-windows-static`（静态 CRT）、`x86-windows`、`arm64-osx`。它同时决定依赖库的 ABI；一个目录一套安装。自定义 triplet（overlay-triplets）用自定义 vcpkg triplet 文件覆盖默认编译选项。

## 版本控制
manifest 模式下用 `builtin-baseline`（指向 vcpkg 仓库某 commit）+ 每依赖版本约束（`>=`/`version>=` 等）+ `overrides` 实现「锁文件」语义；`vcpkg x-update-baseline` 更新基线。

## 双重缓存
- binary caching：编译产物按 ABI 哈希缓存（默认本地，可接 GitHub Actions Cache/NuGet 等），换项目/换 CI 免重编——解决「源码构建慢」的痛点
- asset caching：源码压缩包下载的镜像缓存（CI/内网场景）

## registries 与 overlay
- registries：自定义 ports 集合（git 仓库形式），manifest 中声明来源；可托管私有依赖
- overlay ports/triplets：临时替换/追加个别 port 或 triplet，不改 ports tree（自制 port 的调试路径：`vcpkg add port` 生成骨架 → overlay 调试 → 提交或入 registry）

## 常用命令速记
`bootstrap-vcpkg`（初始化）；`vcpkg search/add port <name>`（查/建配方）；`vcpkg install`（manifest 模式在项目根执行）；`vcpkg list/upgrade`；`vcpkg x-update-baseline`；`--triplet` / 环境变量 `VCPKG_DEFAULT_TRIPLET`。

## 待深入清单
- [ ] portfile.cmake 全貌：fetch 源码 → patch → vcpkg_cmake_configure/build/install → vcpkg_cmake_config_fixup
- [ ] versioning.db 与 builtin-baseline 的解析机制
- [ ] binary caching 后端配置（默认缓存路径、CI 缓存读写策略）
- [ ] vcpkg 与 Conan 对比（同为源码构建哲学 vs 二进制优先哲学）
- [ ] manifest features 与 CMake option 的联动（VCPKG_MANIFEST_FEATURES）→ 见 `integration.md`
