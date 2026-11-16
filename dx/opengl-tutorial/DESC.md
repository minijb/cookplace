---
id: opengl-tutorial
title: OpenGL 教学（含 Compute Shader）
brief: 面向 OpenGL 与 Compute Shader 初学者的系统完整教学路线，基础概念、关键步骤与首次出现的 API 需稍详细讲解
tags: [opengl, graphics, glsl, compute-shader, tutorial]
aliases: [opengl教学, OpenGL教学, opengl系统教程, 学opengl进阶, 计算着色器教学, compute shader教学]
father: tutorial-authoring
---
# OpenGL 教学（含 Compute Shader）

## 是什么
一套现代 OpenGL 与 Compute Shader 的系统教学大纲。OpenGL 部分从基础概念与环境搭建开始，覆盖进阶渲染、现代 API、调试和性能；Compute Shader 部分从零讲解执行模型、数据资源、并行同步和图形管线协作。

完整课程列表见 `dx:opengl-tutorial/knowledge/curriculum.md`，联网调研与外部来源见 `dx:opengl-tutorial/reference/research-sources.md`。

## 作用
用于安排学习顺序、生成后续教程和检查阶段成果。目标读者按 OpenGL 与 Compute Shader 初学者对待，不预设已有图形学、OpenGL 或并行计算经验。教程需要稍微详细一些：首次出现的概念、关键操作步骤、API 参数与数据流向都应解释清楚，同时避免无信息量的重复铺垫。

## 与父级的关系
本 dx 是 `tutorial-authoring` 的子单元，将父级的教程调研、范围确认、目录结构和排版规范落实到 OpenGL 教学主题。本单元定义具体课程内容，不重新定义通用教程制作规范。

## 版本基线
- 基础与进阶渲染以 OpenGL 3.3 Core Profile 为起点。
- Compute Shader 使用 OpenGL 4.3 Core Profile 或支持等价扩展的环境。
- Direct State Access（DSA）等现代 API 作为 OpenGL 4.5 专题学习，不把 OpenGL 4.6 当作全部课程的硬性门槛。
