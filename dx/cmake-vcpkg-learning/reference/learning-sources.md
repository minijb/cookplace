---
title: 学习资源调研来源
brief: 本 dx 学习内容的外部来源汇总：CMake/vcpkg 官方文档、经典书与社区教程，2026-11-14 调研并验证可达性；2026-11-16 围绕 VSCode 集成实战二次补充并抓取核实
source: 见正文各条 URL
fetched: 2026-11-14
---
# 学习资源调研来源

2026-11-14 调研。可达性已用 curl 验证（HTTP 200），**页面内容要点未逐一核实，深入学习前请访问原文确认**。GitHub 域名在当前网络不可达（状态 000），相关条目仅凭常识收录、标注「未验证」。

2026-11-16 二次调研：围绕 VSCode + CMake + vcpkg 集成实战补充下表新增条目，页面内容已实际抓取核实（标注日期）；GitHub 文档条目本次经搜索抓取到实际内容，升级为已验证。

## CMake

| 用途 | 来源 | 状态 | 说明 |
|---|---|---|---|
| 官方交互式教程（主教程） | https://cmake.org/cmake/help/latest/guide/tutorial/index.html | 已验证 | 官方 step-by-step，阶段 1–4 的主线。**2026-11-14 二次核实：4.x 时代已整体重构为 Step 0–11 新体系（Getting Started → Language → Cache → Target → Library → Introspection → Custom Commands → CTest → Install → Finding Dependencies → Misc），旧版 Step 1–12 体系已弃用** |
| 官方手册总入口 | https://cmake.org/cmake/help/latest/ | 常识收录 | 最新文档当前为 CMake 4.5.0 |
| CMake Presets 官方手册 | https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html | 已验证 | CMakePresets.json 权威定义，阶段 4 主线 |
| FetchContent 官方模块文档 | https://cmake.org/cmake/help/latest/module/FetchContent.html | 已验证 | 依赖三方案之一的权威参考 |
| An Introduction to Modern CMake（免费书） | https://cliutils.gitlab.io/modern-cmake/ | 已验证 | 社区公认的「现代 CMake」入门书，目标/属性优先哲学 |
| Professional CMake（Craig Scott，付费书） | https://crascit.com/professional-cmake/ | 已验证 | 页面显示已出第 22 版；CMake 深入阶段的事实标准书 |
| CGold（免费书，较老但概念讲解好） | https://cgold.readthedocs.io/en/latest/ | 已验证 | 三阶段模型/变量作用域等概念性内容讲得透；版本偏旧需对照官方文档 |
| CPM.cmake | https://github.com/cpm-cmake/CPM.cmake | 未验证（GitHub 不可达） | FetchContent 封装，自动缓存；依赖三方案的补充选项 |
| CMake 最佳实践模板仓库 | https://github.com/cpp-best-practices/cmake_template | 未验证（GitHub 不可达） | 可直接抄的工程化模板（presets/测试/警告策略） |
| Visual Studio 17 2022 生成器文档 | https://cmake.org/cmake/help/latest/generator/Visual%20Studio%2017%202022.html | 已验证（2026-11-16） | `-A` 平台与 `-T` toolset 语义；VS 生成器编译器由 toolset 决定，显式 cl.exe 无效 |
| CMake Tools presets 集成文档 | https://github.com/microsoft/vscode-cmake-tools/blob/main/docs/cmake-presets.md | 已验证（2026-11-16，内容已抓取） | `cmake.useCMakePresets` 取值 always/never/auto 及 VSCode 侧 presets 驱动方式 |

## vcpkg

| 用途 | 来源 | 状态 | 说明 |
|---|---|---|---|
| vcpkg 官方文档总入口 | https://learn.microsoft.com/en-us/vcpkg/ | 已验证 | Microsoft Learn 托管的官方文档站 |
| 上手教程 | https://learn.microsoft.com/en-us/vcpkg/get_started/get-started | 已验证 | 阶段 5 主线：bootstrap → hello world |
| Manifest 模式 | https://learn.microsoft.com/en-us/vcpkg/consume/manifest-mode | 已验证 | vcpkg.json / vcpkg_installed/ / 版本约束，阶段 5–6 核心 |
| Triplets 概念 | https://learn.microsoft.com/en-us/vcpkg/concepts/triplets | 已验证 | 三元组与 ABI 语义，阶段 6 核心 |
| Ports 概念 | https://learn.microsoft.com/en-us/vcpkg/concepts/ports | 已验证 | port 配方与 ports tree 结构 |
| 版本控制 | 见 manifest-mode 页内章节 | 已验证（并入 manifest 页） | 独立 versioning 页已 404，版本约束内容在 manifest 文档中 |
| Binary Caching | https://learn.microsoft.com/en-us/vcpkg/users/binarycaching | 已验证 | 二进制缓存机制与后端配置 |
| Asset Caching | https://learn.microsoft.com/en-us/vcpkg/users/assetcaching | 已验证 | 源码包镜像缓存（CI/内网） |
| Registries | https://learn.microsoft.com/en-us/vcpkg/users/registries | 已验证 | 自定义 ports 集合 / 私有依赖托管 |
| Overlay Ports | https://learn.microsoft.com/en-us/vcpkg/concepts/overlay-ports | 已验证 | 自制 port 的调试路径 |
| CMake 集成官方说明 | https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration | 已验证 | toolchain 接入与自动安装，阶段 7 主线 |
| vcpkg 官网/仓库 | https://vcpkg.io / https://github.com/microsoft/vcpkg | 官网已验证 / GitHub 未验证 | 官网为 JS 应用；仓库含完整 ports 源可读 |
| VSCode 官方上手教程 | https://learn.microsoft.com/en-us/vcpkg/get_started/get-started-vscode | 已验证（2026-11-16，内容已抓取） | VCPKG_ROOT 环境变量 + CMakePresets 注入 toolchain 的官方步骤；`vcpkg new --application` 生成 manifest + baseline；VSCode 集成实战主线 |
| vcpkg.json 字段参考 | https://learn.microsoft.com/en-us/vcpkg/reference/vcpkg-json | 已验证（2026-11-16） | builtin-baseline 仅顶层 manifest 生效等权威定义，版本锁定依据 |
| CMake 集成官方说明（变量表） | https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration | 已验证（2026-11-16，内容已抓取） | `VCPKG_INSTALLED_DIR` 默认值（manifest: `${CMAKE_BINARY_DIR}/vcpkg_installed`）、`VCPKG_APPLOCAL_DEPS` 默认 ON、toolchain 在 `project()` 求值 |
| glad port 页 | https://vcpkg.io/en/package/glad | 已验证（2026-11-16） | `gl-api-43`/`loader` 等特性名确认，OpenGL 项目依赖实测 |

## 版本速览（调研时点）
- CMake 官方文档 `latest` = 4.5.0（2026-11 查）
- Professional CMake 22nd Edition（作者页 2026-11 查）
- vcpkg 文档随 Microsoft Learn 持续更新，无固定版本号；以 builtin-baseline 锁定行为
