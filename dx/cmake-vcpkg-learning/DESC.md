---
id: cmake-vcpkg-learning
title: CMake + vcpkg 学习
brief: C++ 构建与依赖管理双工具：CMake 目标模型/预设/FetchContent 与 vcpkg 清单模式/三元组/缓存/注册表，及两者工具链集成
tags: [cmake, vcpkg, cpp, learning, build-system, package-manager]
aliases: [cmake学习, 学cmake, vcpkg学习, 学vcpkg, cmake, vcpkg, 构建系统, 依赖管理, 包管理]
father: tutorial-authoring
---
# CMake + vcpkg 学习

## 是什么
C++ 工具链学习中「构建 + 依赖」方向的知识单元：CMake（构建系统生成器）与 vcpkg（微软 C++ 依赖管理器）双工具，以两者集成为最终目标。学习者画像沿用 cpp-learning：前端游戏开发工程师，日常 JS/TS 工具链（npm/webpack/vite），用类比方式消化 C++ 侧概念。

## 作用
- 学习路径：按 `knowledge/roadmap.md` 七阶段推进（CMake 基础 → 目标与属性 → 依赖与子项目 → 预设工程化 → vcpkg 入门 → vcpkg 深入 → 集成实战）
- 知识域分组：
  - CMake：`knowledge/cmake-core.md`
  - vcpkg：`knowledge/vcpkg-core.md`
  - 两者集成（toolchain/find_package/VSCode 实战/交叉编译）：`knowledge/integration.md`
- 外部资源：`reference/learning-sources.md`（2026-11-14 调研，URL 已验证可达性；2026-11-16 围绕 VSCode 集成实战二次补充并核实内容）

## 与父级的关系
父 dx 为 tutorial-authoring（教程制作规范）。本 dx 是其在「CMake + vcpkg 学习」领域的具体化：产出遵循父 dx 的排版与目录结构规范。父 dx 管「怎么写」，本 dx 管「学什么、学多深」。

兄弟 dx 为 cpp-learning（同为 tutorial-authoring 子级）：cpp-learning 管语言与引擎，本 dx 管构建与依赖，为其提供工程化支撑；学习顺序建议在 cpp-learning 阶段 1 并行起步。

## 与子级的关系
暂无子 dx。若某知识域（如 vcpkg registries、交叉编译）内容膨胀，可拆为子 dx。
