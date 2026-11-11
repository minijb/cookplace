---
title: STL 容器内部结构
brief: vector/string/deque/map/unordered_map/smart pointers 的实现级结构与开销，含 allocator 与 pmr 接入点
tags: [cpp, stl, memory]
---
# STL 容器内部结构

实现级事实以 libstdc++ / libc++ 为主参照，数值类细节标注「实现相关，以本地实现为准」。权威参考：cppreference 与各实现源码；对照表见 `dx:cpp-learning/reference/learning-sources.md`。

## 序列容器
- `vector`：三指针（begin/end/cap）。增长因子实现相关（GCC/Clang 约 2×、MSVC 约 1.5×）；扩容重分配使全部迭代器失效；C++11 起搬移元素用移动构造——但仅当移动构造是 `noexcept` 才敢用。
- `string`：SSO 短串栈内联（内联容量实现相关：libc++ 22B、libstdc++/MSVC 15B 上下），长串走堆；C++11 起标准禁止 COW。
- `deque`：中央 map 数组索引固定大小的 chunk（libstdc++ 512B/chunk）；两端 O(1) 且不搬已有元素；随机访问需跨 chunk 算偏移，迭代器比 vector 重。
- `list`：双向链表 node（prev/next/value），元素地址稳定但逐个堆分配、遍历缓存不友好。

## 关联容器
- `map` / `set`：红黑树节点（颜色/父/左右子/值），每元素独立分配，节点开销大；有序遍历，操作 O(log n)。
- `unordered_map` / `unordered_set`：桶数组 + 链式节点（libstdc++ 用 before_begin 哨兵）；负载因子超限触发 rehash——**迭代器失效但元素指针/引用仍有效**；平均 O(1)，最差 O(n)。
- 游戏热路径替代思路：有序 vector + 二分、开放寻址哈希（如 absl/robin_hood 风格）。

## 智能指针与函数对象
- `shared_ptr`：控制块存强/弱引用计数与 deleter；`make_shared` 单次分配合并对象与控制块（缓存友好 + 少一次分配），但 weak 长存会拖住整块内存。
- `unique_ptr`：零额外开销（无状态 deleter 时）。
- `function`：小缓冲优化（SBO，容量实现相关，libc++ 约 24B），放不下的 callable 堆分配；调用有间接跳转，热路径慎用。

## 值包装
- `optional` / `variant`：栈上存储 + 判别标签，零堆分配；`variant` visit 经函数表跳转。

## allocator 与 pmr（C++17）
- 容器的第二模板参数 allocator 是池化的接入点。
- `pmr::monotonic_buffer_resource`（只增不减的 arena）+ `pmr::vector`：帧分配器的标准做法；`pmr::synchronized_pool_resource` 通用池。
- pmr 容器类型与默认 allocator 容器**类型不同**，接口边界要注意。

## 待深入
- 各实现源码精读（libstdc++ bits/stl_vector.h）、EBO 在 tuple/compressed pair 中的应用、boost::flat_map 类紧凑容器
