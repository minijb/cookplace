---
title: C++17 游戏开发进阶
series: cpp17-game-dev
topic: 面向前端游戏开发工程师的 C++17 深入教程
audience: 已掌握类、指针、引用、虚函数和基本 STL 的前端游戏开发工程师
outcome: 逐步掌握现代 C++、内存与所有权、STL、异步任务系统、UE 和图形 API
environment: C++17；CMake 3.16+；GCC、Clang 或 MSVC；Windows + MSVC 19.50 已验证；2026-11-18
date: 2026-11-18
---
# C++17 游戏开发进阶

本系列面向前端游戏开发工程师，以 C++17 为基线，通过可运行代码和实现级分析建立现代 C++ 知识框架。

## 你将学会什么

- 用 RAII、值语义和智能指针表达资源生命周期与所有权。
- 深入 STL、对象布局、内存模型、并发和异步任务系统。
- 将语言知识连接到游戏循环、ECS、Unreal Engine 和图形 API。

## 学习路线

1. 现代语法与资源管理。
2. STL 深入。
3. 模板、并发与异步。
4. 游戏开发专题。
5. Unreal Engine 与图形 API。
6. 对象布局、语言与并发内存模型、STL 内部结构和性能优化贯穿全系列。

## 第一部分：现代语法与资源管理

| 课 | 内容 | 配套代码 |
|---|---|---|
| [第一章（上）：现代语法与 RAII](part-01-modern-cpp/01-modern-syntax-and-raii.md) | `auto`、结构化绑定、`optional`、`variant`、`string_view`、RAII | [源码](code/01-modern-syntax-and-raii/main.cpp) |
| [第一章（下）：移动语义与智能指针](part-01-modern-cpp/02-move-semantics-and-smart-pointers.md) | 值类别、Rule of Zero/Five、移动操作、智能指针、资源缓存与性能测量 | [源码](code/02-move-semantics-and-smart-pointers/main.cpp) |

## 第二部分：STL 深入

| 课 | 内容 | 配套代码 |
|---|---|---|
| [第二章（上）：顺序容器与内存行为](part-02-stl/01-sequence-containers.md) | 容器选型、`size/capacity`、扩容、`noexcept` 与元素搬移、缓存行为 | [源码](code/03-sequence-containers/main.cpp) |
| [第二章（中）：关联容器、哈希与适配器](part-02-stl/02-associative-containers.md) | 有序树、严格弱序、哈希桶与再哈希、C++17 插入接口、节点句柄、适配器 | [源码](code/04-associative-containers/main.cpp) |
| [第二章（下）：迭代器、算法与 lambda](part-02-stl/03-iterators-algorithms.md) | 迭代器类别、算法契约、lambda 捕获、remove-erase、失效规则、`string_view` | [源码](code/05-iterators-algorithms/main.cpp) |
| [第二章（实战）：游戏实体存储与性能测量](part-02-stl/04-game-entity-storage.md) | 实体存储与交换删除、索引方案对比、微基准、`pmr`、并行算法 | [源码](code/06-game-entity-storage/main.cpp) |

## 第三部分：模板、并发与异步

| 课 | 内容 | 配套代码 |
|---|---|---|
| [第三章（上）：模板基础与实参推导](part-03-concurrency-async/01-templates.md) | 实参推导与 decay、CTAD 与推导指引、非类型模板参数、重载决议、代码组织 | [源码](code/07-templates/main.cpp) |
| [第三章（中）：变参模板与编译期分支](part-03-concurrency-async/02-compile-time.md) | 参数包、折叠表达式、`if constexpr`、`void_t` 探测、`enable_if`/SFINAE、tag dispatch | [源码](code/08-compile-time/main.cpp) |
| [第三章（中下）：线程与数据竞争](part-03-concurrency-async/03-threads.md) | RAII 守护、数据竞争与 TSan 现场、mutex 家族、死锁防护、条件变量生产者-消费者 | [源码](code/09-threads/main.cpp)、[竞争演示](code/09-threads/race_demo.cpp) |
| [第三章（下）：原子操作与内存序](part-03-concurrency-async/04-atomics.md) | atomic 与 CAS、六档 `memory_order`、relaxed 计数、acquire/release 配对、自旋锁、false sharing 实测 | [源码](code/10-atomics/main.cpp) |
| [第三章（实战）：异步与线程池](part-03-concurrency-async/05-async-thread-pool.md) | future 四件套、async 三大陷阱、异常传递、手写 mutex + CV + packaged_task 线程池、帧末 join | [源码](code/11-async-thread-pool/main.cpp) |

## 第四部分：游戏开发专题

| 课 | 内容 | 配套代码 |
|---|---|---|
| [第四章（上）：游戏循环与时间](part-04-game-dev/01-game-loop-and-time.md) | 固定步长 + 累加器、整数刻度防漂移、死亡螺旋钳制、渲染插值纪律、与 rAF 对照 | [源码](code/12-game-loop/main.cpp) |
| [第四章（中）：实体架构与 ECS](part-04-game-dev/02-entity-architecture.md) | 继承崩坏、实体句柄（索引 + 代数）、sparse set 池、view 与遍历纪律、EnTT 对照 | [源码](code/13-ecs/main.cpp)、[EnTT 示例](code/13-ecs/entt_demo.cpp) |
| [第四章（中下）：游戏模式精选](part-04-game-dev/03-game-patterns.md) | Update Method、Observer/Event Queue、Object Pool、Spatial Partition（含对角格坑） | [源码](code/14-game-patterns/main.cpp) |
| [第四章（下）：对象生命周期与句柄化](part-04-game-dev/04-object-lifecycle.md) | 索引 + 代数句柄表、帧分配器（placement new/析构）、关卡 RAII 与卸载顺序 | [源码](code/15-lifecycle-handles/main.cpp) |
| [第四章（实战）：任务系统](part-04-game-dev/05-job-system.md) | 依赖 DAG、fork/join 与 `parallel_for`、帧同步点纪律、work stealing 方向、三陷阱 | [源码](code/16-job-system/main.cpp) |

## 构建并验证示例

需要 CMake 3.16 或更高版本，以及支持 C++17 的编译器。在仓库根目录执行：

```bash
cmake -S tutorials/cpp17-game-dev/code -B .tmp/cpp17-game-dev-build
cmake --build .tmp/cpp17-game-dev-build --config Debug
ctest --test-dir .tmp/cpp17-game-dev-build -C Debug --output-on-failure
```

`--config Debug` 和 `ctest` 的 `-C Debug` 用于 Visual Studio 等多配置生成器；使用 Ninja 等单配置生成器时可以省略。

第三部分新增说明：`code/third_party/entt/` 内置 EnTT v3.13.2 头文件（供 `ch04_entt_demo` 使用，无网络依赖）；`code/09-threads/race_demo.cpp` 生成 `ch03_race_demo` 可执行文件，供 TSan 手动运行，故意不注册进 CTest。

本系列调研与范围确认记录见 [第一章调研记录](resources/ch01-research.md)、[第二章调研记录](resources/ch02-research.md)、[第三章调研记录](resources/ch03-research.md)、[第四章调研记录](resources/ch04-research.md)。

## 相关专题

- [C++17 RAII 与异常安全](../cpp17-raii/README.md)：对象生命周期、构造失败、析构、所有权和异常安全。
- [C++17 字符串使用与优化](../cpp17-strings/README.md)：字符串接口、UTF-8、内存机制、`pmr` 和性能测量。
