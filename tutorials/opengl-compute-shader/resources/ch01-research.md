---
title: 第一章调研与确认记录
series: opengl-compute-shader
chapter: 第一章
topic: 初学者课程导览的范围、概念准确性与来源记录
audience: 教程作者与审校者
outcome: 追溯首课为何先讲 OpenGL 职责、渲染管线、核心术语和 Compute 定位
environment: 在线资料调研；2026-11-16
date: 2026-11-16
---
# 第一章调研与确认记录

## 范围确认

- 系列总大纲：`dx:opengl-tutorial/knowledge/curriculum.md`。
- 用户已确认完整大纲，并随后明确要求按初学者对待、讲解稍微详细一些。
- 本章对应第 0 部第 1 课：学前准备与课程导览。
- 用户本次明确要求创建第一章教程，因此不再重复请求范围确认。
- 本章不包含安装步骤、窗口创建代码或三角形代码；这些内容分别属于后续课程。
- 为形成完整课程入口，同时创建系列 `README.md`，但不提前撰写其他课正文。

## 调研要点

1. LearnOpenGL 明确面向无图形编程经验的读者，采用线性章节结构，并要求一定的 C++ 基础；数学知识可在需要时补充。这支持“概念导览 + 顺序学习 + 按需数学”的首课结构。
2. Khronos FAQ 将 OpenGL 定义为描述光栅化渲染系统行为的规范，由硬件厂商驱动实现；同时明确 OpenGL 不负责动画、计时、文件 I/O、图像格式和 GUI。因此首课需要讲清职责边界。
3. Khronos Getting Started 将初始化区分为 Context 创建和函数加载，并说明窗口工具库的作用。本章只介绍该关系，具体步骤留给第三章。
4. Khronos Rendering Pipeline Overview 给出 Vertex Processing、可选 Tessellation/Geometry、裁剪、光栅化、Fragment Shader 和 Per-sample Processing。本章使用明确标注的简化管线，避免让初学者误以为可选阶段不存在。
5. OpenGL 4.6 Core 规范与 GLSL 4.60 规范说明 Compute Processor 独立于其他 Shader Processor；Compute Shader 不属于图形管线，副作用通过 Image、Storage Buffer 和 Atomic Counter 等产生。因此首课用“更新粒子 + 渲染粒子”的例子区分 Compute 与渲染。

## 来源与核实标记

| 来源 | 用途 | 核实状态 |
|---|---|---|
| [LearnOpenGL Introduction](https://learnopengl.com/Introduction) | 初学者定位、C++ 先修要求、数学讲解和线性课程结构 | 已检索并读取摘要，2026-11-16 |
| [Khronos OpenGL FAQ](https://www.khronos.org/opengl/wiki/FAQ) | OpenGL 规范/驱动定义、职责边界与 Context | 已检索并读取摘要，2026-11-16 |
| [Khronos Getting Started](https://wikis.khronos.org/opengl/Getting_Started) | Context、函数加载、窗口工具库与渲染循环 | 已检索并读取摘要，2026-11-16 |
| [Khronos Rendering Pipeline Overview](https://wikis.khronos.org/opengl/Rendering_Pipeline_Overview) | 渲染阶段顺序、Fragment 与像素的区别 | 已检索并读取摘要，2026-11-16 |
| [OpenGL 4.6 Core Profile Specification](https://registry.khronos.org/OpenGL/specs/gl/glspec46.core.pdf) | OpenGL 执行模型和 Compute Processor 定位 | 已检索并读取摘要，2026-11-16 |
| [GLSL 4.60.8 Specification](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html) | Compute Shader 独立于图形管线、Workgroup 和资源副作用 | 已检索并读取摘要，2026-11-16 |
| [Khronos Compute Shader Wiki](https://wikis.khronos.org/opengl/Compute_Shader) | Compute 自 OpenGL 4.3 进入 Core、Dispatch 与 Workgroup 概览 | 已检索并读取摘要，2026-11-16 |

## 大纲与写作检查

- 首课覆盖：OpenGL 定义与边界、CPU/GPU/驱动/Shader 分工、简化图形管线、Triangle/Shader/Texture/MVP/Camera、Compute 对比、课程路线、学习准备与自测。
- 图形关系均配 Mermaid 图，图前后保留文字说明；简化管线明确说明省略可选阶段。
- 没有引入需要提前掌握的 Barrier、SSBO 布局或 API 代码细节。
- 所有外部资料均在正文“参考资料”中给出链接。

## 验证状态

- 本课为纯概念课，不包含终端命令或可执行示例代码，无需构建验证。
- 版本结论仅使用已核实的 OpenGL 3.3 Core 教学起点和 Compute Shader 4.3 Core 要求；未声称任何具体设备已经验证。
- 未使用截图或外部图片。
