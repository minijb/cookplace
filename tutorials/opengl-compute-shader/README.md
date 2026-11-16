---
title: OpenGL 与 Compute Shader 系统教程
series: opengl-compute-shader
topic: 从现代 OpenGL 入门、进阶渲染到 Compute Shader 与图形管线协作
audience: 不预设图形学或 OpenGL 经验、希望系统学习渲染与 GPU 计算的初学者
outcome: 能独立编写和调试 OpenGL 渲染程序，并完成 Compute Shader 驱动的粒子或 GPU Driven 场景
environment: C++17；OpenGL 3.3 Core 起步；Compute Shader 需要 OpenGL 4.3；概念课不要求安装环境
date: 2026-11-16
---
# OpenGL 与 Compute Shader 系统教程

本系列面向初学者，从第一个三角形逐步进入光照、模型、阴影、PBR、性能分析和 Compute Shader。课程不会假设你已经掌握图形学或并行计算；首次出现的概念、关键步骤和 API 会配合示例稍微详细地解释。

完整课程规划见 `dx:opengl-tutorial/knowledge/curriculum.md`。

## 你将学会什么

- 理解现代 OpenGL 的渲染管线、资源对象、Shader 和常用渲染技术。
- 独立实现模型场景、阴影、后处理、Deferred Rendering 和 PBR/IBL。
- 使用 Debug Output、RenderDoc 和 Timer Query 分析正确性与性能。
- 理解 Compute Shader 的 Invocation、Workgroup、SSBO、Image、Shared Memory、Barrier 和 Atomic。
- 完成 Compute Shader 与 OpenGL 渲染协作的综合项目。

## 学习路线

```mermaid
flowchart LR
  A[入门与数学] --> B[OpenGL 基础]
  B --> C[光照与模型]
  C --> D[高级 OpenGL]
  D --> E[高级渲染与 PBR]
  E --> F[调试与性能]
  F --> G[Compute 基础]
  G --> H[同步与并行算法]
  H --> I[综合项目]
```

## 第 0 部：入门、环境与必要数学

| 课 | 内容 | 状态 |
|---|---|---|
| [第一章：课程导览——从第一个三角形到 GPU 计算](part-00-orientation/01-course-overview.md) | OpenGL 的职责、CPU/GPU 分工、图形管线、核心术语、Compute Shader 与课程路线 | 已写 |
| [第二章：现代 OpenGL、立即模式与 Profile](part-00-orientation/02-modern-opengl.md) | 区分立即模式、固定功能管线、现代 OpenGL 与 Core/Compatibility Profile | 已写 |
| [第三章：窗口、OpenGL Context 与渲染主循环](part-00-orientation/03-window-context-loop.md) | 使用 GLFW + GLAD 创建 3.3 Core 窗口并理解主循环 | 已写 |
| [第四章：CMake 工程、依赖与目录结构](part-00-orientation/04-cmake-project.md) | 用 CMakePresets + vcpkg Manifest 构建可重复工程 | 已写 |
| [第五章：向量、矩阵、点积、叉积与齐次坐标](part-00-orientation/05-math-foundations.md) | 通过 GLM 数值示例建立后续变换所需数学基础 | 已写 |
| [第六章：坐标空间、右手系与 MVP 变换链](part-00-orientation/06-coordinate-spaces.md) | 理解 Local、World、View、Clip、NDC 和 Window Space | 已写 |
| [第七章：Debug Output、错误检查与开发日志](part-00-orientation/07-debug-output.md) | 建立 OpenGL 4.3 同步 Debug Output 与错误报告机制 | 已写 |

Part 00 的七章已全部完成。第 3～7 章示例均已在 Windows 11 + MSVC 环境构建并通过 CTest，具体范围和限制见验证记录。

## 后续部分

1. 第 1 部：OpenGL 基础——管线、Shader、VAO/VBO/EBO、Texture、MVP、Camera、Depth Test。
2. 第 2 部：光照、材质与模型场景。
3. 第 3 部：高级 OpenGL 与离屏渲染。
4. 第 4 部：高级光照、阴影与 PBR。
5. 第 5 部：现代 OpenGL、工程化、调试与性能。
6. 第 6 部：Compute Shader 零基础入门。
7. 第 7 部：Compute 数据资源与布局。
8. 第 8 部：并行协作、同步与原子操作。
9. 第 9 部：Compute 性能、图形协作与综合项目。
10. 第 10 部：扩展专题与能力边界。

## 阅读约定

- 按顺序学习；已有经验可以通过每课自测决定是否加快阅读，但不要跳过不熟悉的概念。
- 每课按照“背景与目的 → 原理 → 示例 → 逐步讲解 → 实验 → 常见错误 → 练习”组织。
- 前两课是概念课；第 3～7 章提供[独立可运行示例](code/README.md)，并单独记录构建与测试状态。
- 调研记录：[第一章](resources/ch01-research.md)、[第二章](resources/ch02-research.md)、[第三至第七章](resources/part00-remaining-research.md)。
