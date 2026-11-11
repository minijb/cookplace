---
title: C++ 内存模型
brief: 两层含义拆解：语言对象模型（存储期/生命周期/memory location）与并发内存模型（happens-before、memory_order）
tags: [cpp, memory, concurrency]
---
# C++ 内存模型

「内存模型」在 C++ 里有两层含义，学习时先分清再深入。

## A. 语言层对象模型（单线程语义）
- 对象 = 存储区中的一块区域，具有类型、生命周期、地址。
- 四种存储期：static / thread / automatic / dynamic；对应创建与销毁时机。
- 生命周期：构造开始 → 析构结束；访问生命周期外的对象是 UB。
- memory location：标量对象或相邻非零宽位域序列——这是「数据竞争」定义的基本单位。
- 存储重用：placement new 在既有存储上构造；重用后的可见性涉及 `std::launder`（见待深入）。
- 严格别名（TBAA）：以另一类型的表达式访问对象是 UB（`char`/`unsigned char` 例外）；编译器据此做激进优化。

## B. 并发内存模型（C++11 起）
- 三种顺序关系：sequenced-before（线程内）、synchronizes-with（线程间，由 atomic 操作或锁建立）、happens-before = 前两者的传递闭包。
- 数据竞争：两线程对**同一 memory location** 的冲突访问且无 happens-before 排序 → 整个程序 UB。
- `memory_order` 六档（cppreference 为准）：
  - `seq_cst`（默认）：全序，最直观最贵。
  - `release` 写 + `acquire` 读配对：release 前的所有写，对 acquire 后的读可见——跨线程传数据的标配。
  - `acq_rel`：读写都参与的场合（如 RMW）。
  - `relaxed`：只保证该操作自身原子，不提供同步；适合纯计数。
  - `consume`：不要用，实现上退化为 acquire。
- 游戏场景映射：job system 提交任务 = release store 任务指针，worker = acquire load；共享计数用 relaxed + 独立同步。

## 观测与验证工具
- TSan（-fsanitize=thread）查数据竞争；Relacy/loom 式模型检验（进阶）。

## 待深入
- `std::launder` 与指针透明性、fences（atomic_thread_fence）、 Out-of-Thin-Air 问题的现实影响
