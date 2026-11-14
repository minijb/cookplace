---
title: 学习路径（CMake + vcpkg）
brief: 七阶段学习路径：CMake 基础/目标/依赖/预设 → vcpkg 入门/深入 → 集成实战，含阶段产出
tags: [cmake, vcpkg, learning, roadmap]
---
# 学习路径（CMake + vcpkg）

学习者画像：前端游戏开发工程师（类比锚点：CMake ≈ webpack/vite 的「配置即代码」，vcpkg ≈ npm + node_modules，vcpkg.json ≈ package.json，vcpkg_installed/ ≈ node_modules/，manifest 模式 ≈ npm install 自动化，binary caching ≈ npm cache）。

| 阶段 | 主题 | 目标 | 产出（回写本 dx） |
|---|---|---|---|
| 1 | CMake 基础 | 安装；CMakeLists.txt 跑通 hello-world；理解「配置期→生成期→构建期」三阶段；out-of-source 构建 | 回填 `cmake-core.md` 模型节 |
| 2 | 目标与属性 | add_library/add_executable；target_include_directories/target_link_libraries；PRIVATE/PUBLIC/INTERFACE 传播语义；变量 vs 目标属性 | 回填 `cmake-core.md` 目标节 |
| 3 | 依赖与子项目 | find_package(模块/配置模式)；FetchContent；add_subdirectory；三方取舍 | 回填 `cmake-core.md` 依赖节 |
| 4 | 预设与工程化 | CMakePresets.json；生成器与多配置（Debug/Release）；编译选项与 warning 管理 | 回填 `cmake-core.md` 预设节 |
| 5 | vcpkg 入门 | bootstrap；classic vs manifest 模式；vcpkg.json 常用命令（install/search/list） | 回填 `vcpkg-core.md` 基础节 |
| 6 | vcpkg 深入 | triplets；baseline 与版本约束；binary/asset caching；registries；overlay ports；自制 port | 回填 `vcpkg-core.md` 深入节 |
| 7 | 集成实战 | CMAKE_TOOLCHAIN_FILE 接入；CMake 构建自动触发 vcpkg 安装；CI（GitHub Actions）中组合使用；交叉编译一条龙 | 回填 `integration.md` |

执行方式：
- 每阶段以一个可运行小项目收尾（阶段 7 的产出即最终目标：一个 vcpkg 管依赖 + CMakePresets 驱动的跨平台库/可执行项目）；练习代码放独立代码目录，不进 dx
- 笔记回写对应 knowledge 文件的「待深入清单」逐步消化；确认的结论上移为正文
- 外部资源按 `reference/learning-sources.md` 中调研的官方/社区教程展开
