---
title: STL 深入
brief: 容器选型与复杂度、迭代器失效规则、算法与 lambda 配合
tags: [cpp, stl]
---
# STL 深入

## 容器选型
- `vector` 默认首选：连续内存、缓存友好；`deque` 两端增删；`list` 仅在需要稳定地址或大量中间插删时用。
- `map`（红黑树、有序）vs `unordered_map`（哈希、均摊 O(1)）；游戏热路径常用「vector + 排序」或开放寻址哈希替代。
- 字符串传参用 `string_view`（C++17），注意其不拥有数据。

## 迭代器失效
- `vector` 扩容使全部迭代器/指针/引用失效；各容器插入、删除的失效规则不同，一律回查 cppreference 确认，不凭记忆。

## 算法与 lambda
- `<algorithm>` 优先于手写循环：sort / find_if / transform / accumulate；remove-erase 惯用法。
- 泛型 lambda（auto 参数）；C++17 没有 ranges，算法组合靠循环串联（C++20 ranges 列入展望）。

## 待深入
- 自定义 allocator 与内存池结合、C++17 并行算法（execution policy）
