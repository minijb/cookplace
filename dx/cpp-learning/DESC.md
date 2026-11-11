---
id: cpp-learning
title: C++ 学习（前端游戏开发方向）
brief: 面向前端游戏开发工程师的 C++17 深入学习：语言核心与对象布局、内存模型、STL 内部结构、异步任务系统、UE 与图形 API
tags: [cpp, learning, gamedev, ue, graphics]
aliases: [C++学习, 学C++, cpp, c++, c++学习]
father: tutorial-authoring
---
# C++ 学习（前端游戏开发方向）

## 是什么
面向「前端游戏开发工程师」角色的 C++ 深入学习知识单元。学习者已具备 C++ 基础（类、指针、引用、虚函数、基本 STL 使用），以 C++17 为基线向深入推进。

角色视角：前端游戏开发工程师出身——日常工作在 Web 游戏/引擎前端（JS/TS、Canvas/WebGL），深入 C++ 的目的是读懂引擎（尤其 Unreal Engine）源码、编写高性能模块与异步任务系统。

## 作用
- 学习路径：按 `dx:cpp-learning/knowledge/roadmap.md` 的五阶段推进
- 知识域分组：
  - 语言核心：`core-language.md`；内存底层：`object-layout.md`（类内存结构）、`memory-model.md`（语言/并发内存模型）、`stl-internals.md`（容器内部）、`memory-performance.md`（优化）
  - STL：`stl.md`；异步与任务：`async.md`、`job-system.md`；游戏开发专题：`game-dev.md`
  - UE：`unreal-engine.md`；图形 API：`graphics-api.md`
- 学习产出落库：确认的结论按 `dx:cpp-learning/spec/note-template.md` 回写 knowledge/；杂项（技巧/坑/片段）按 `dx:cpp-learning/spec/misc-template.md` 写入项目 `misc/cpp/`
- 外部资源：`dx:cpp-learning/reference/learning-sources.md`

## 与父级的关系
父 dx 为 tutorial-authoring（教程制作规范）。本 dx 是其在「C++ 学习」领域的具体化：产出内容遵循父 dx 的排版、目录结构（含 misc/ 杂项归档）与 mermaid 图表规范。父 dx 管「怎么写」，本 dx 管「学什么、学多深」。

## 与子级的关系
暂无子 dx。若某知识域（如 UE、图形 API）内容膨胀，可拆为子 dx。
