---
title: 第二章调研与确认记录
series: cpp17-game-dev
topic: STL 容器、迭代器、算法与游戏实体存储的教程调研
audience: 教程作者与审校者
outcome: 追溯第二章的范围、章节依据、权威来源和代码验证状态
environment: 在线资料调研；2026-11-13
date: 2026-11-13
---
# 第二章调研与确认记录

## 范围确认

- 初始路线：`dx:cpp-learning/knowledge/roadmap.md` 第二阶段，以及 `dx:cpp-learning/knowledge/stl.md`。
- 用户确认范围：容器、内存、算法与迭代器全部覆盖。
- 用户确认深度：实现级深入。
- 沿用环境：C++17、跨平台 CMake，示例接入 CTest。
- 用户确认结构：第二章拆成顺序容器、关联容器、迭代器与算法、游戏实体存储实战四课。
- 经确认的补充内容：容器适配器、迭代器类别、严格弱序、哈希与再哈希、异常安全、`pmr`、C++17 并行算法和性能测量。

## 调研要点

1. 主流 STL 教程通常按容器、迭代器、算法组织；游戏开发教程还需要把容器选择连接到连续内存、指针追逐、分配频率与帧预算。
2. 容器内部布局大多是实现细节，不应把三指针、红黑树节点或具体增长倍数写成所有标准库都遵守的布局保证。正文优先讲标准化行为、复杂度和失效规则，再用“常见实现”解释原因。
3. 迭代器、指针和引用的失效规则并不相同。`vector` 重分配会全部失效；无序容器再哈希会使迭代器失效，但元素指针和引用仍然有效。
4. 容器选型不能只看大 O：连续内存、每元素开销、访问模式、是否要求有序、地址稳定性与删除频率都影响结果。
5. 算法正确性依赖迭代器类别和前置条件，例如排序需要随机访问迭代器，二分查找要求范围按键有序；比较器必须满足严格弱序。
6. `remove`/`remove_if` 只移动保留元素并返回新逻辑末尾，不会自行缩小容器，因此需要与 `erase` 配合。
7. `string_view` 是非拥有视图，适用于只读参数，但不延长底层字符生命周期，也不保证 `data()` 后以空字符结尾。
8. `pmr` 将容器与内存资源解耦；单调资源适合集中释放的临时分配，但资源与上游必须比使用它们的容器活得更久。
9. C++17 执行策略表达允许的执行方式，不保证每次调用一定并行；工具链支持和后端依赖需要实测。
10. 基准测试不预设排名，不把当前环境的一次测量推广成固定加速比；排序向量、哈希表和链表应分别匹配各自的访问场景。

## 来源与核实标记

| 来源 | 用途 | 核实状态 |
|---|---|---|
| [cppreference：容器库](https://en.cppreference.com/cpp/container) | 容器分类、复杂度和失效规则总表 | 已检索，2026-11-13 |
| [C++17 标准草案：容器要求](https://timsong-cpp.github.io/cppwp/std17/containers) | 标准层面的容器与失效语义 | 已检索，2026-11-13 |
| [cppreference：`std::vector`](https://en.cppreference.com/cpp/container/vector) | 连续存储、容量、重分配和移动/复制选择 | 已检索，2026-11-13 |
| [C++ Core Guidelines：标准库容器](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines.html) | 默认容器与连续内存建议 | 已检索，2026-11-13 |
| [cppreference：`std::unordered_map`](https://en.cppreference.com/cpp/container/unordered_map) | 桶、负载因子、平均与最差复杂度 | 已检索，2026-11-13 |
| [cppreference：`try_emplace`](https://en.cppreference.com/cpp/container/unordered_map/try_emplace) | C++17 条件插入和不覆盖语义 | 已检索，2026-11-13 |
| [cppreference：`extract`](https://en.cppreference.com/cpp/container/unordered_map/extract) | C++17 节点句柄 | 已检索，2026-11-13 |
| [cppreference：算法库](https://en.cppreference.com/cpp/algorithm) | 算法契约、迭代器要求和执行策略 | 已检索，2026-11-13 |
| [cppreference：`std::remove`](https://en.cppreference.com/cpp/algorithm/remove) | remove-erase 行为 | 已检索，2026-11-13 |
| [cppreference：`std::sort`](https://en.cppreference.com/cpp/algorithm/sort) | 随机访问迭代器和比较器要求 | 已检索，2026-11-13 |
| [cppreference：`std::transform`](https://en.cppreference.com/cpp/algorithm/transform) | 一元与二元变换语义 | 已检索，2026-11-13 |
| [cppreference：`std::accumulate`](https://en.cppreference.com/cpp/algorithm/accumulate) | 初值类型与折叠语义 | 已检索，2026-11-13 |
| [cppreference：`std::string_view`](https://en.cppreference.com/cpp/string/basic_string_view) | 非拥有视图与失效规则 | 已检索，2026-11-13 |
| [cppreference：`std::pmr::memory_resource`](https://en.cppreference.com/cpp/memory/memory_resource) | 多态内存资源接口 | 已检索，2026-11-13 |
| [cppreference：执行策略](https://en.cppreference.com/cpp/algorithm/execution_policy_tag_t) | 顺序、并行和向量化执行策略语义 | 已检索，2026-11-13 |
| [Microsoft C++ Team Blog：C++17 并行算法](https://devblogs.microsoft.com/cppblog/using-c17-parallel-algorithms-for-better-performance/) | 并行候选、线程安全检查与实现说明 | 已检索，2026-11-13 |
| [LearnCpp：STL 容器](https://www.learncpp.com/cpp-tutorial/stl-containers-overview/) | 主流教程的容器分类与组织 | 已检索，2026-11-13 |
| [LearnCpp：STL 迭代器](https://www.learncpp.com/cpp-tutorial/stl-iterators-overview/) | 迭代器概念与章节组织 | 已检索，2026-11-13 |
| [LearnCpp：STL 算法](https://www.learncpp.com/cpp-tutorial/stl-algorithms-overview/) | 算法与迭代器的组合方式 | 已检索，2026-11-13 |
| [Game AI Pro：游戏引擎中的 STL 容器](http://www.gameaipro.com/GameAIProOnlineEdition2021/GameAIProOnlineEdition2021_Chapter15_Should_STL_containers_be_used_in_game_engines.pdf) | 游戏引擎的容器选型与常见疑问 | 已检索，2026-11-13 |
| [EASTL：最佳实践](https://github.com/electronicarts/EASTL/blob/master/doc/BestPractices.md) | 游戏开发中的分配与连续存储经验 | 已检索，2026-11-13；作为第三方经验，不替代标准语义 |

## 验证状态

- 2026-11-13：使用 Windows、CMake 4.2.1、MSVC 19.50（Visual Studio 18 2026 生成器）完成配置、构建与测试。
- 全部 10 个 CTest 目标通过，含第二章 7 个新目标：`ch02_sequence_containers`、`ch02_associative_containers`、`ch02_iterators_algorithms`、`ch02_entity_storage`、`ch02_stl_benchmark`、`ch02_pmr`、`ch02_parallel_algorithms`。
- `CH02_HAS_PARALLEL_ALGORITHMS` 编译探测在 MSVC 19.50 下成功，`std::reduce(std::execution::par, ...)` 正常编译运行。
- Debug 构建基准示例输出（仅示例，不代表其他环境）：vector 遍历 37 us、list 遍历 600 us、有序 vector 二分查询 19098 us、unordered_map 查询 10486 us；两种查询路径校验值一致。
- 验证中发现并修复：含中文注释且无 BOM 的源文件触发 MSVC C4819，且曾使 `ch02_entity_storage` 断言失败；为源文件添加 UTF-8 BOM 后问题消失，已归档为杂项 `misc/cpp/01-pitfall-msvc-c4819-bom.md`。
- GCC、Clang 与其他操作系统未在当前环境验证；正文不声称已覆盖这些工具链。
