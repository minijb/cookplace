---
title: 第二章调研与确认记录
series: opengl-compute-shader
chapter: 第二章
topic: 现代 OpenGL、立即模式、固定功能管线和 Profile 的概念核实
audience: 教程作者与审校者
outcome: 追溯本章的版本历史、概念边界、教学取舍和权威来源
environment: 在线资料调研；2026-11-16
date: 2026-11-16
---
# 第二章调研与确认记录

## 范围确认

- 本章对应 `dx:opengl-tutorial/knowledge/curriculum.md` 第 0 部第 2 课。
- 用户已确认系列完整大纲，并在完成第一章后明确要求开始下一章。
- 本章只讲概念和教程辨别方法，不创建 Context，不提供窗口或三角形代码。
- 版本和 Profile 的具体创建步骤留给第三章，避免本章同时承担概念课与环境课两种目标。

## 调研要点

1. Khronos Legacy OpenGL 将 `glBegin`/`glEnd` 视为立即模式顶点属性指定的典型信号，并单独列出矩阵栈和固定功能光照，支持本章把“数据提交”与“处理逻辑”分开讲解。
2. Khronos Fixed Function Pipeline 提醒：现代管线仍保留不可编程状态，“固定功能管线”通常特指被 Shader 替代的旧式处理机制。因此正文明确避免“现代 OpenGL 全部可编程”的误解。
3. Khronos Deprecation 与 OpenGL Context 给出历史节点：3.0 引入弃用，3.1 移除大量功能，3.2 引入 Core/Compatibility Profile；Compatibility Context 可恢复被 Core 移除的功能。
4. OpenGL 3.3 Core/Compatibility 规范分别定义两种 Profile 的功能集合，说明版本号和 Profile 必须一起描述。
5. LearnOpenGL 以 OpenGL 3.3 Core 教授现代基础，并指出后续版本增加功能而不改变核心机制，支持本课程的教学起点。
6. Compute Shader 自 OpenGL 4.3 起进入 Core Profile，因此 3.3 只作为渲染基础起点，不作为 Compute 实验环境。

## 来源与核实标记

| 来源 | 用途 | 核实状态 |
|---|---|---|
| [Khronos Legacy OpenGL](https://wikis.khronos.org/opengl/Legacy_OpenGL) | 立即模式、内置矩阵、固定功能光照和常见旧 API | 已检索并读取摘要，2026-11-16 |
| [Khronos Fixed Function Pipeline](https://wikis.khronos.org/opengl/Fixed_Function_Pipeline) | 固定功能概念及现代管线仍有固定状态的边界 | 已检索并读取摘要，2026-11-16 |
| [Khronos Deprecation](https://wikis.khronos.org/opengl/Deprecation) | OpenGL 3.0/3.1 弃用与移除历史 | 已检索并读取摘要，2026-11-16 |
| [Khronos OpenGL Context](https://wikis.khronos.org/opengl/OpenGL_Context) | OpenGL 3.2 Profile、Core/Compatibility Context 和平台差异 | 已检索并读取摘要，2026-11-16 |
| [Khronos History of OpenGL](https://wikis.khronos.org/opengl/History_of_OpenGL) | 弃用功能和版本历史的交叉核对 | 已检索并读取摘要，2026-11-16 |
| [OpenGL 3.3 Core Profile Specification](https://registry.khronos.org/OpenGL/specs/gl/glspec33.core.pdf) | Core 规范、弃用模型和 GLSL 3.30 | 已检索并读取摘要，2026-11-16 |
| [OpenGL 3.3 Compatibility Profile Specification](https://registry.khronos.org/OpenGL/specs/gl/glspec33.compatibility.pdf) | Compatibility 规范及其与 Core 的差异 | 已检索并读取摘要，2026-11-16 |
| [Khronos Compute Shader Wiki](https://wikis.khronos.org/opengl/Compute_Shader) | Compute Shader 的 OpenGL 4.3 Core 版本要求 | 已检索并读取摘要，2026-11-16 |
| [LearnOpenGL Introduction](https://learnopengl.com/Introduction) | 现代 OpenGL 与 3.3 Core 的教学路线 | 已检索并读取摘要，2026-11-16 |

## 大纲与写作检查

- 明确区分立即模式与固定功能管线，没有将二者写成同义词。
- 明确区分 Deprecated、Removed、版本号和 Profile。
- 明确现代 OpenGL 仍包含光栅化、Depth Test、Blending 等固定功能或可配置阶段。
- 没有声称 Core Profile 自动带来性能提升，也没有声称 Compatibility 等于旧版本。
- 所有旧 API 只以行内名称用于识别，不提供需要运行验证的示例代码。
- 正文为初学者解释每个首次出现的对象，并把具体创建步骤留到后续课程。

## 验证状态

- 本课为纯概念课，不包含终端命令或可执行示例代码，无需构建验证。
- 版本历史由 Khronos Wiki 与对应官方规范交叉核对。
- 未声称任何具体设备或驱动已经验证，也未使用截图或外部图片。
