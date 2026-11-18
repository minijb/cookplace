---
title: 异步编程模型
brief: C++17 的 future/promise/async 机制与陷阱、异常传递、与多线程和任务系统的关系，C++20 协程展望
tags: [cpp, async, concurrency]
---
# 异步编程模型

异步 ≠ 多线程：异步描述「发起后不等待结果」，实现可以靠线程、事件循环或协程。C++17 标准库的异步原语只有 future/promise 一族。

## std::future 一族（C++11，C++17 沿用）
- 生产者：`std::promise` set_value / set_exception；消费者：`future::get`（一次性，get 后失效；多处消费用 `shared_future`）。
- `std::async`：把可调用体变成 future。launch policy：`std::launch::async`（确定开线程）vs `deferred`（惰性，get 时才在调用线程执行）；默认 `async|deferred` 由实现裁量。
- `std::packaged_task`：把可调用体包成可手动执行、结果自动送进 future 的对象，适合自建任务队列。

## 关键陷阱
- `std::async` 返回的 future 析构时会**阻塞等待任务完成**（标准规定的 join 语义）——future 提前离开作用域等于同步调用。实测确认（教程 part-03）：两个各睡 100ms 的任务以「丢弃返回值」方式发起，总耗时 200ms 完全串行。MSVC /W4 会对丢弃 async 返回值发 C4858 警告。
- `deferred` 的 future 析构则不执行任务（任务被丢弃）。
- 默认 policy 下是否真的开线程由实现裁量，行为不可控；帧内高频路径慎用 std::async。
- 手写线程池的实测结构（教程 part-03）：`packaged_task` 做类型擦除、`submit` 返回 future、worker 谓词版 wait、退出条件「stop 且队列空」（析构不丢任务）、锁外执行任务。

## 异常传递
- 任务线程的未捕获异常经 promise 存入 future，`get()` 时在消费线程重抛；不 get 就永久吞掉——异步路径必须显式消费结果。

## 与任务系统的关系
- std::async 每次可能新建线程，粒度太重；帧内并行应使用任务系统（依赖 DAG + work stealing），见 `dx:cpp-learning/knowledge/job-system.md`；UE 侧对应 TaskGraph / Tasks System（见 `dx:cpp-learning/knowledge/unreal-engine.md`）。

## C++20 协程（展望）
- co_await / co_yield / co_return：用户态挂起与恢复，异步逻辑写成同步风格；与 JS 的 await 解决同一类问题，但实现是编译器状态机改写。
- 基线外内容：对照 cppreference 协程条目与 cppcoro 等社区库。

## 待深入
- work stealing 无锁队列实现、executors 提案动向、Asio 的异步运行时模型
