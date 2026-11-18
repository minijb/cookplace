---
title: 第三章调研与确认记录
series: cpp17-game-dev
topic: 模板、线程、原子与内存序、异步与线程池的教程调研
audience: 教程作者与审校者
outcome: 追溯第三章的范围、章节依据、权威来源和代码验证状态
environment: 在线资料调研；2026-11-18
date: 2026-11-18
---
# 第三章调研与确认记录

## 范围确认

- 初始路线：`dx:cpp-learning/knowledge/roadmap.md` 第三阶段，以及 `core-language.md` 模板节、`memory-model.md`、`async.md`。
- 用户确认深度（2026-11-18）：模板部分取「实用深度 + 元编程基础」——函数/类模板、实参推导、CTAD、变参模板与折叠表达式、`if constexpr`，并补 type traits 手写、`enable_if`/SFINAE 家族与 tag dispatch。
- 用户确认实战形式（2026-11-18）：「都要」——并发收尾用 mutex + condition_variable + future 手写线程池，并加 TSan 数据竞争现场演示。
- 沿用环境：C++17、跨平台 CMake，示例接入 CTest；TSan 演示程序编译但不入 CTest（含数据竞争的 UB 程序不应进自动测试）。
- 经确认的章节结构：模板基础 → 编译期编程（变参/fold/if constexpr/traits/SFINAE）→ 线程与数据竞争 → 原子与内存序 → 异步与线程池实战，共五课。

## 调研要点

1. 主流模板教程（LearnCpp、C++ Templates 2e、C++17 - The Complete Guide）按「函数模板 → 实参推导 → 类模板 → 变参 → 编译期分支」组织；C++17 的推荐实践是用 `if constexpr` 与 CTAD 取代大部分经典 SFINAE 写法，但 SFINAE 仍是读懂旧代码与引擎源码的必备知识。
2. 模板推导的关键区分：按值传递会 decay（数组退化为指针、顶屋 const 丢失），按引用传递保留引用性；`auto` 与模板推导规则一致。
3. 并发教学共识（Effective Modern C++ Item 35、C++ Concurrency in Action、abseil 提醒）：优先任务式而非线程式；`std::atomic` 六档中 `seq_cst` 是默认且最直观，`release` 写 + `acquire` 读是跨线程传数据的标配，`consume` 不应使用；非专家不要手写无锁结构。
4. `std::async` 三大坑在多个来源反复出现：返回的 future 析构会阻塞等待任务完成（丢弃 future 等于同步调用）；默认 launch policy `async|deferred` 由实现裁量，可能完全不并发；`deferred` 的 future 析构不执行任务。
5. 异常传递：任务线程的未捕获异常存入 shared state，`get()` 时在消费线程重抛；不消费则永久吞掉。
6. 手写线程池的标准最小形态：worker 循环 + 任务队列 + mutex + condition_variable + stop 标志；`submit` 用 `packaged_task` 包裹可调用体返回 `future`。生产实现还需处理队列有界、任务丢弃、worker 主动偷取等，教程明确其教学定位。
7. 数据竞争是 UB，肉眼不可靠，教学应展示 TSan 现场报告；GCC/Clang 用 `-fsanitize=thread`，MSVC 自 Visual Studio 2022 起以实验特性支持 `/fsanitize=thread`（x64），不可用时以文档说明代替演示输出。
8. `false sharing`：不同线程频繁写同一 cache line 的不同变量会引发一致性流量；用 `alignas(std::hardware_destructive_interference_size)`（或保守的 64）分隔计数器演示。`hardware_destructive_interference_size` 在部分实现上尚未稳定，教程代码用固定 64 并注明。

## 来源与核实标记

| 来源 | 用途 | 核实状态 |
|---|---|---|
| [cppreference：模板推导](https://en.cppreference.com/w/cpp/language/template_argument_deduction) | 实参推导与 decay 规则 | 已检索，2026-11-18 |
| [LearnCpp：模板章节](https://www.learncpp.com/cpp-tutorial/template-classes/) | 主流教程的模板章节组织 | 已检索，2026-11-18 |
| [C++ Templates: The Complete Guide 2e（目录）](https://www.oreilly.com/library/view/c-templates-the/9780134778808/ch17.xhtml) | 变参模板与 fold expressions 章节划分 | 已检索，2026-11-18 |
| [C++17 - The Complete Guide（目录）](https://www.cppstd17.com/cppstd17.html) | C++17 模板增量：CTAD、推导指引、compile-time if | 已检索，2026-11-18 |
| [cppreference：`if constexpr`](https://en.cppreference.com/w/cpp/language/if) | 编译期分支语义 | 已检索，2026-11-18 |
| [cppreference：fold expressions](https://en.cppreference.com/w/cpp/language/fold) | 一元/二元折叠语义 | 已检索，2026-11-18 |
| [cppreference：`std::enable_if`](https://en.cppreference.com/w/cpp/types/enable_if) | SFINAE 经典写法 | 已检索，2026-11-18 |
| [cppreference：`std::thread`](https://en.cppreference.com/w/cpp/thread/thread) | joinable/join/detach 契约 | 已检索，2026-11-18 |
| [cppreference：`std::condition_variable`](https://en.cppreference.com/w/cpp/thread/condition_variable) | wait/notify 与虚假唤醒 | 已检索，2026-11-18 |
| [cppreference：`std::memory_order`](https://en.cppreference.com/w/cpp/atomic/memory_order) | 六档 memory_order 标准定义 | 已检索，2026-11-18 |
| [abseil：The Danger of Atomic Operations](https://abseil.io/docs/cpp/atomic_danger) | 非专家不要手写无锁结构的社区共识 | 已检索，2026-11-18 |
| [Effective Modern C++ 第 7 章（目录）](https://www.oreilly.com/library/view/effective-modern-c/9781491908419/ch07.html) | 任务式优于线程式、async 与 launch policy | 已检索，2026-11-18 |
| [Scott Meyers：std::futures from std::async aren't special](http://scottmeyers.blogspot.com/2013/03/stdfutures-from-stdasync-arent-special.html) | async future 析构阻塞的标准依据 | 已检索，2026-11-18 |
| [Concurrency Ninja：Futures 与 Pitfalls Catalog](https://p0w.github.io/concurrency-ninja/ch07-futures.html) | 丢弃 future、默认 policy、deferred 丢弃不执行 | 已检索，2026-11-18 |
| [C++ Concurrency in Action 2e 第 4 章（目录）](https://livebook.manning.com/learn/c-plus-plus-concurrency-in-action-second-edition/chapter-4) | future/promise/packaged_task 体系 | 已检索，2026-11-18 |
| [Arm Learning Paths：C++ Memory Model and Atomics](https://learn.arm.com/learning-paths/servers-and-cloud-computing/arm-cpp-memory-model/2/) | 原子与内存序的硬件视角 | 已检索，2026-11-18 |
| [MSVC 文档：ThreadSanitizer 支持](https://learn.microsoft.com/cpp/build/reference/fsanitize) | MSVC `/fsanitize=thread` 实验特性与限制 | 已检索，2026-11-18 |
| [Community 教程：C++ 线程池最小实现](https://cpplearn.com/article/building-a-basic-thread-pool) | mutex+cv+packaged_task 线程池标准形态 | 已检索，2026-11-18 |

## 验证状态

- 教程代码目录 `code/07-templates` 至 `code/11-async-thread-pool`，已随写作全部构建并接入 CTest。
- 2026-11-18：Windows + MSVC 19.50 干净构建零警告，全部用例通过；`ch03_race_demo`（含数据竞争 UB）编译但不注册测试。
