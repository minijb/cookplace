---
title: OpenGL 与 Compute Shader 系统教程
series: opengl-compute-shader
topic: 从现代 OpenGL 入门、进阶渲染到 Compute Shader 与图形管线协作
audience: 不预设图形学或 OpenGL 经验、希望系统学习渲染与 GPU 计算的初学者
outcome: 能独立编写和调试 OpenGL 渲染程序，并完成 Compute Shader 驱动的粒子或 GPU Driven 场景
environment: C++17；OpenGL 3.3 Core 起步；Compute Shader 需要 OpenGL 4.3；概念课不要求安装环境
date: 2026-11-18
---
# OpenGL 与 Compute Shader 系统教程

本系列面向初学者，从窗口与数学基础进入三角形、纹理、MVP、Camera，再逐步学习光照、模型、阴影、PBR、性能分析和 Compute Shader。首次出现的概念、关键步骤和 API 会配合可运行示例与图示解释。

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
| [第 1 课：课程导览——从第一个三角形到 GPU 计算](part-00-orientation/01-course-overview.md) | OpenGL 的职责、CPU/GPU 分工、图形管线、核心术语、Compute Shader 与课程路线 | 已写 |
| [第 2 课：现代 OpenGL、立即模式与 Profile](part-00-orientation/02-modern-opengl.md) | 区分立即模式、固定功能管线、现代 OpenGL 与 Core/Compatibility Profile | 已写 |
| [第 3 课：窗口、OpenGL Context 与渲染主循环](part-00-orientation/03-window-context-loop.md) | 使用 GLFW + GLAD 创建 3.3 Core 窗口并理解主循环 | 已写 |
| [第 4 课：CMake 工程、依赖与目录结构](part-00-orientation/04-cmake-project.md) | 用 CMakePresets + vcpkg Manifest 构建可重复工程 | 已写 |
| [第 5 课：向量、矩阵、点积、叉积与齐次坐标](part-00-orientation/05-math-foundations.md) | 通过 GLM 数值示例建立后续变换所需数学基础 | 已写 |
| [第 6 课：坐标空间、右手系与 MVP 变换链](part-00-orientation/06-coordinate-spaces.md) | 理解 Local、World、View、Clip、NDC 和 Window Space | 已写 |
| [第 7 课：Debug Output、错误检查与开发日志](part-00-orientation/07-debug-output.md) | 建立 OpenGL 4.3 同步 Debug Output 与错误报告机制 | 已写 |

Part 00 的 7 课已全部完成。第 3～7 课示例的既有验证范围见对应调研记录。

## 第 1 部：OpenGL 基础快速重建

| 课 | 内容 | 示例与交互 |
|---|---|---|
| [第 1 课：图形管线全景与 OpenGL 状态机](part-01-basics/01-pipeline-state.md) | 从顶点数据追踪到 Fragment 与 Framebuffer | Clear Color 状态实验 |
| [第 2 课：Shader 编译与链接](part-01-basics/02-shader-compilation.md) | Vertex/Fragment Shader、Program 与诊断日志 | 编译链接并绘制三角形 |
| [第 3 课：VBO、VAO 与 Vertex Attribute](part-01-basics/03-vbo-vao-attributes.md) | 数据存储、属性格式与绘制状态 | 上传位置并绘制三角形 |
| [第 4 课：EBO 与索引绘制](part-01-basics/04-ebo-indexed-drawing.md) | 索引复用、`glDrawElements` 与 VAO 绑定 | 矩形；`L` 切换线框 |
| [第 5 课：Hello Triangle 与插值](part-01-basics/05-hello-triangle-interpolation.md) | 交错属性、阶段接口与重心插值 | 彩色渐变三角形 |
| [第 6 课：GLSL 基础](part-01-basics/06-glsl-foundations.md) | 类型、限定符、Swizzle 与内置函数 | 程序化条纹三角形 |
| [第 7 课：Uniform、时间与交互](part-01-basics/07-uniforms-interaction.md) | Uniform Location、更新和输入 | 方向键移动、时间变色 |
| [第 8 课：Texture 与采样](part-01-basics/08-textures-sampling.md) | UV、Sampler、Wrap、Filter 与 Mipmap | `F`/`W` 切换采样状态 |
| [第 9 课：多纹理与 Texture Unit](part-01-basics/09-multiple-textures.md) | 两个 Sampler、单元绑定与混合 | 方向键调整混合比例 |
| [第 10 课：Transform 与 GLM](part-01-basics/10-transforms-glm.md) | 平移、旋转、缩放与矩阵顺序 | 旋转矩形；空格暂停 |
| [第 11 课：Model、View、Projection](part-01-basics/11-mvp.md) | MVP、透视除法、裁剪与 Viewport | 透视旋转立方体 |
| [第 12 课：Camera、输入与帧时间](part-01-basics/12-camera-delta-time.md) | FPS Camera、鼠标、WASD 与 `deltaTime` | 可移动三维场景 |
| [第 13 课：Depth Test、Face Winding 与 Culling](part-01-basics/13-depth-culling.md) | 深度遮挡、绕序与背面剔除 | `D`/`C`/`F` 切换状态 |

Part 01 的 13 课全部提供正文、独立 CMake 示例和原创 SVG 原理图。所有示例均可通过 CTest 执行隐藏窗口冒烟测试；交互与视觉效果仍建议按课文手动运行确认。

## 后续部分

1. 第 2 部：光照、材质与模型场景（待编写）。
2. 第 3 部：高级 OpenGL 与离屏渲染。
3. 第 4 部：高级光照、阴影与 PBR。
4. 第 5 部：现代 OpenGL、工程化、调试与性能。
5. 第 6 部：Compute Shader 零基础入门。
6. 第 7 部：Compute 数据资源与布局。
7. 第 8 部：并行协作、同步与原子操作。
8. 第 9 部：Compute 性能、图形协作与综合项目。
9. 第 10 部：扩展专题与能力边界。

## 阅读约定

- 按顺序学习；已有经验可以通过每课练习检查是否真正掌握，不要只运行复制代码。
- 每课按照“背景与目的 → 原理 → 示例 → 逐步讲解 → 实验 → 常见错误 → 独立练习”组织。
- 可运行代码集中在 [code/](code/)，每课有独立 CMake 工程；共享头文件和测试资产的用途见 [代码运行说明](code/README.md)。
- 图片集中在 [assets/](assets/)。Part 01 的 SVG 为本项目原创，可直接离线查看，不依赖外部图片链接。
- 教程网址与扩展阅读集中在 [resources/README.md](resources/README.md)；调研和验证记录也保存在该目录。

## 调研记录

- [第 0 部第 1 课](resources/ch01-research.md)
- [第 0 部第 2 课](resources/ch02-research.md)
- [第 0 部第 3～7 课](resources/part00-remaining-research.md)
- [第 1 部全部课程](resources/part01-research.md)
