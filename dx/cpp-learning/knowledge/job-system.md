---
title: 游戏任务系统（Job System）
brief: 帧内并行任务系统设计：任务图与依赖、work stealing、帧同步点、数据划分与陷阱
tags: [cpp, gamedev, async, concurrency]
---
# 游戏任务系统（Job System）

游戏引擎的自建异步基础设施：把一帧的 CPU 工作拆成小任务并行跑，在帧内关键点 join。UE 的 TaskGraph / Tasks System、Unity Job System 同属此模型。

## 核心概念
- 任务 = 小粒度可调用体 + 依赖；任务间形成有向无环图（DAG），依赖满足才可调度。
- 调度器：每个 worker 线程一个任务队列，空闲线程从他人队列尾部偷任务（work stealing）——减少锁竞争、天然负载均衡。
- fork/join 模式：并行拆分 → 依赖汇聚处 join；join 点是读共享结果的唯一合法时机。

## 与游戏循环的协作
- 帧内并行点：动画更新、粒子、物理宽相位、剔除、资产解压与 IO。
- 同步点纪律：游戏线程只在 join 后读权威状态；渲染提交是每帧最大的 join 点。
- 跨帧任务（异步加载等）走独立低优先级队列，不占帧预算。

## 数据划分与内存配合
- 粒度：太细调度开销吞掉收益，太粗负载不均；以「单任务 ≥ 数十微秒」起判，实测校准。
- 写分离：并行任务可共享读、不共享写；写冲突靠依赖串行化。
- 配合 SoA 数据布局与 cache line 对齐减少 false sharing（见 `dx:cpp-learning/knowledge/memory-performance.md`）。
- 任务内禁止无界分配：预分配结果缓冲或用帧分配器。

## 与内存序的配合
- 提交/完成标志用 acquire/release 原子操作传递可见性（见 `dx:cpp-learning/knowledge/memory-model.md`）；调度器内部是无锁队列的重灾区。

## 典型陷阱
- join 嵌套死锁：worker 内同步等待另一任务，应改为依赖声明或 continuation。
- 忘记 join → 读到未完成数据；任务线程异常被吞 → 仿 future 的异常传递机制（见 `dx:cpp-learning/knowledge/async.md`）。

## UE 对应实现
- TaskGraph：上一代 job manager；Tasks System（UE5）官方定位「improves on the TaskGraph」，两者共用同一调度后端（调度器 + worker 线程）：prerequisites 依赖 DAG、嵌套任务（父任务随嵌套全部完成才完成）、Pipe 串行链（近似 green thread）、五档任务优先级；UE5.5 起 oversubscription（唤醒 standby 线程）替代 busy-waiting。ParallelFor 做循环并行。详见 `dx:cpp-learning/knowledge/unreal-engine.md`。
- 官方纪律：等待会损害扩展性，优先用任务依赖建图而非显式 Wait——与本文「join 点纪律」一致。

## 待深入
- Chase-Lev work stealing 队列实现、任务优先级与饿死、structured concurrency
