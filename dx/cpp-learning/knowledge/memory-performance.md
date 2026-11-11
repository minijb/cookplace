---
title: 内存与性能优化
brief: 内存模型、对象布局、缓存友好数据设计与剖析方法
tags: [cpp, memory, performance]
---
# 内存与性能优化

## 内存
- 三种存储期：静态/自动/动态；栈分配近零成本，堆分配昂贵且有碎片问题。
- 对象布局：对齐与 padding 影响大小，成员排列有讲究；虚函数引入 vptr 与虚表成本。
- 内存池/对象池：游戏开发常规手段，规避运行时分配抖动；配合 placement new 定点构造。
- 高发错误：悬垂指针、use-after-free、double-free、迭代器失效——RAII 是第一道防线。

## 性能方法
- 先测量后优化：profiler（perf / VTune / Tracy）定位热点，禁止凭感觉优化。
- 缓存友好：数据布局决定性能——SoA vs AoS 的取舍、热数据连续存放、减少指针追逐与虚调用。
- 编译选项：-O2/-O3、LTO；`inline` 是请求不是命令。
- 减少拷贝：移动语义、`emplace_back`、容器 `reserve` 预分配。

## 游戏场景特有
- 帧预算思维：60fps 即每帧 16.6ms，理想状态是帧内零堆分配。
- 帧分配策略：arena/帧分配器（帧末整体重置）、双缓冲。

## 优化工具箱
- 分配策略：对象池/内存池、arena（`pmr::monotonic_buffer_resource`）、帧分配器；容器 `reserve` 一次到位，`shrink_to_fit` 非强制。
- 布局优化：热/冷字段分离、SoA 改造、按 cache line（64B）对齐并隔离高频写字段，避免 false sharing。
- 减少 rehash/重分配：`unordered_map` 预留桶、`vector` 预留容量；有序 vector + 二分替代 node 型关联容器。
- 拷贝消除：移动语义、`emplace_back`、RVO/NRVO（C++17 保证 prvalue 消除）；`make_shared` 合并分配。
- 检测工具：ASan（-fsanitize=address）查越界/UAF、TSan 查竞争、Valgrind/heap profiler 查泄漏与分配热点、Tracy 看帧内分配。

底层支撑：对象布局见 `dx:cpp-learning/knowledge/object-layout.md`，语言/并发内存模型见 `dx:cpp-learning/knowledge/memory-model.md`，容器内部结构与开销见 `dx:cpp-learning/knowledge/stl-internals.md`。

## 待深入
- 无锁数据结构（基于 acquire/release 语义）、SIMD、分支预测友好写法、自定义 operator new 全局接管
