---
title: 学习路径（C++17 深入）
brief: 面向前端游戏开发工程师的五阶段学习路径与阶段产出
tags: [cpp, learning]
---
# 学习路径（C++17 深入）

学习者画像：前端游戏开发工程师，已掌握 C++ 基础（类、指针、引用、虚函数、基本 STL），以 C++17 为基线深入。

| 阶段 | 主题 | 目标 | 产出（回写本 dx） |
|---|---|---|---|
| 1 | 现代语法与资源管理 | auto/结构化绑定/optional/variant/string_view；RAII、智能指针、Rule of Five、移动语义 | 回填 `core-language.md`、`memory-performance.md` |
| 2 | STL 深入 | 容器选型与复杂度、迭代器失效、算法与 lambda、string_view 传参 | 回填 `stl.md` |
| 3 | 模板、并发与异步 | 函数/类模板、if constexpr；thread/atomic/memory_order 入门；future/promise/async 机制与陷阱 | 回填 `core-language.md` 并发节、`memory-model.md`、`async.md` |
| 4 | 游戏开发专题 | 游戏循环、ECS、对象生命周期、异步任务系统 | 回填 `game-dev.md`、`job-system.md` |
| 5 | UE 专题与图形 API | UE 对象模型/GC/Gameplay 框架/TaskGraph；渲染管线概念 | 回填 `unreal-engine.md`、`graphics-api.md` |
| 贯穿 | 内存底层专题 | 对象/类内存结构、语言与并发内存模型、STL 容器内部结构、内存优化手段 | 回填 `object-layout.md`、`memory-model.md`、`stl-internals.md`、`memory-performance.md` |

执行方式：
- 每阶段以编码练习收尾；练习代码放独立代码目录，不进 dx
- 笔记与杂项按模板回写：`dx:cpp-learning/spec/note-template.md`、`dx:cpp-learning/spec/misc-template.md`
- 阶段进退看产出物，不看时间；各文件的「待深入清单」是下一轮学习入口
- 路线图参考 `dx:cpp-learning/reference/learning-sources.md` 中调研的外部路线，按本角色裁剪
