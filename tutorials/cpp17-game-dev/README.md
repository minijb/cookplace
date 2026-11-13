---
title: C++17 游戏开发进阶
series: cpp17-game-dev
topic: 面向前端游戏开发工程师的 C++17 深入教程
audience: 已掌握类、指针、引用、虚函数和基本 STL 的前端游戏开发工程师
outcome: 逐步掌握现代 C++、内存与所有权、STL、异步任务系统、UE 和图形 API
environment: C++17；CMake 3.16+；GCC、Clang 或 MSVC；Windows + MSVC 19.50 已验证；2026-11-13
date: 2026-11-13
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
| [第一章（上）：现代语法与 RAII](part-01-modern-cpp/01-modern-syntax-and-raii/index.md) | `auto`、结构化绑定、`optional`、`variant`、`string_view`、RAII | [源码](code/01-modern-syntax-and-raii/main.cpp) |
| [第一章（下）：移动语义与智能指针](part-01-modern-cpp/02-move-semantics-and-smart-pointers/index.md) | 值类别、Rule of Zero/Five、移动操作、智能指针、资源缓存与性能测量 | [源码](code/02-move-semantics-and-smart-pointers/main.cpp) |

## 第二部分：STL 深入

| 课 | 内容 | 配套代码 |
|---|---|---|
| [第二章（上）：顺序容器与内存行为](part-02-stl/01-sequence-containers/index.md) | 容器选型、`size/capacity`、扩容、`noexcept` 与元素搬移、缓存行为 | [源码](code/03-sequence-containers/main.cpp) |
| [第二章（中）：关联容器、哈希与适配器](part-02-stl/02-associative-containers/index.md) | 有序树、严格弱序、哈希桶与再哈希、C++17 插入接口、节点句柄、适配器 | [源码](code/04-associative-containers/main.cpp) |
| [第二章（下）：迭代器、算法与 lambda](part-02-stl/03-iterators-algorithms/index.md) | 迭代器类别、算法契约、lambda 捕获、remove-erase、失效规则、`string_view` | [源码](code/05-iterators-algorithms/main.cpp) |
| [第二章（实战）：游戏实体存储与性能测量](part-02-stl/04-game-entity-storage/index.md) | 实体存储与交换删除、索引方案对比、微基准、`pmr`、并行算法 | [源码](code/06-game-entity-storage/main.cpp) |

## 构建并验证示例

需要 CMake 3.16 或更高版本，以及支持 C++17 的编译器。在仓库根目录执行：

```bash
cmake -S tutorials/cpp17-game-dev/code -B .tmp/cpp17-game-dev-build
cmake --build .tmp/cpp17-game-dev-build --config Debug
ctest --test-dir .tmp/cpp17-game-dev-build -C Debug --output-on-failure
```

`--config Debug` 和 `ctest` 的 `-C Debug` 用于 Visual Studio 等多配置生成器；使用 Ninja 等单配置生成器时可以省略。

本章调研与范围确认记录见 [第一章调研记录](resources/ch01-research.md)、[第二章调研记录](resources/ch02-research.md)。

## 相关专题

- [C++17 RAII 与异常安全](../cpp17-raii/README.md)：对象生命周期、构造失败、析构、所有权和异常安全。
- [C++17 字符串使用与优化](../cpp17-strings/README.md)：字符串接口、UTF-8、内存机制、`pmr` 和性能测量。
