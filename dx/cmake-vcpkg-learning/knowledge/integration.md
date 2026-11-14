---
title: CMake + vcpkg 集成
brief: CMAKE_TOOLCHAIN_FILE 接入机制、manifest 模式与 CMake 构建的联动、CI 与交叉编译组合
tags: [cmake, vcpkg, integration, toolchain]
---
# CMake + vcpkg 集成

## 接入机制：toolchain file
官方推荐方式是把 vcpkg 自带的 `vcpkg.cmake` 作为 CMake 工具链文件：
```
cmake -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake
```
机制：工具链文件在配置期注入 vcpkg 的 `installed/<triplet>/` 到 CMake 的搜索路径链（`CMAKE_PREFIX_PATH` 等）与 `find_package` 的失败回退逻辑，使 find_package 语义不需要逐库写路径。preset 中固化该 `-D`，项目成员零心智负担。

## manifest 模式 × CMake 构建
启用 vcpkg.cmake 且项目根有 vcpkg.json 时，CMake configure 会**自动**运行 vcpkg 安装依赖到构建目录旁的 `vcpkg_installed/`（开关：`-DVCPKG_MANIFEST_INSTALL=OFF`）。即「cmake --preset 即装即建」，npm install 般体验。
- triplet 由 `VCPKG_TARGET_TRIPLET` 传入，默认 `x64-windows`（Windows）/平台默认（其他 OS）
- `VCPKG_MANIFEST_FEATURES` 可把 vcpkg.json 的 features 与 CMake option 打通（如 tests 特性只装测试依赖）

## 与依赖三方案的组合边界
- vcpkg 只解决「第三方库从哪来」；项目结构仍由 CMake target 表达
- 库在 vcpkg 有 port → find_package（经 toolchain）；没有 → FetchContent/CPM；两者可混用
- 官方库没收录时：自制 port（overlay → registry）vs 直接 FetchContent 的取舍——前者可复用缓存与统一 ABI，后者上手快

## CI 组合（GitHub Actions）
要点：缓存 `vcpkg` 二进制缓存目录（GitHub Actions Cache 后端）避免每次源码重编；`lukka/run-vcpkg` 等社区 action 封装了 bootstrap + vcpkg.json 驱动的 CMake 配置；矩阵（os × triplet）各跑一条 preset。

## 交叉编译一条龙
自定义 triplet（目标平台 + sysroot/编译器）→ `-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE` 接平台工具链 → vcpkg 依赖按目标 ABI 构建 → CMake 同链生成目标构建脚本。典型场景：x64 主机交叉构建 arm64 嵌入式/移动目标。

## 待深入清单
- [ ] vcpkg.cmake 注入的完整变量/路径清单（阅读 scripts/buildsystems/vcpkg.cmake 源码）
- [ ] applocal 部署（Windows 下 DLL 复制到输出目录，`add_custom_command(TARGET ... POST_BUILD)` 惯用法）
- [ ] 多 triplet 共存（同项目 x64-windows 调试 + x64-windows-static 发布）的目录策略
- [ ] 与 UE 的关系：UE 自带构建系统不吃 vcpkg，vcpkg 适用于引擎外工具链/独立引擎方向 → 对齐 cpp-learning 学习目标后再深入
