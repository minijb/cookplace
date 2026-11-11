---
title: 图形 API 基础
brief: 渲染管线概念、OpenGL/WebGL/WebGPU 谱系与引擎向进阶入口
tags: [cpp, graphics]
---
# 图形 API 基础

## 渲染管线概念
- 流程：顶点数据 → 顶点着色器 → 光栅化 → 片元着色器 → 输出合并；模型/视图/投影矩阵变换。
- 图形 API 本质是状态机；draw call 次数是主要 CPU 开销来源之一（合批的动机）。

## API 谱系
- OpenGL：跨平台经典，适合入门练习。
- Vulkan：显式、冗长、控制粒度最细，引擎向进阶方向。
- WebGPU：现代显式 API 标准，概念对标 Vulkan/Metal/D3D12（管线、绑定组、命令编码），原生实现有 dawn / wgpu。
- D3D12 / Metal：Windows / Apple 平台私有显式 API。

## C++ 练习路线
- 桌面：GLFW + glad 起步 OpenGL；原生 WebGPU 走 dawn / wgpu。
- 引擎内：UE 的 RHI 层抽象了图形 API，Gameplay 代码通常不直接调用（见 `dx:cpp-learning/knowledge/unreal-engine.md`）。

## 待深入
- 着色器语言（GLSL/WGSL）、资源上传与同步语义、frame graph 概念
