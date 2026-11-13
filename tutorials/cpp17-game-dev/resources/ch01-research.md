---
title: 第一章调研与确认记录
series: cpp17-game-dev
topic: 现代语法、RAII、移动语义与智能指针的教程调研
audience: 教程作者与审校者
outcome: 追溯第一章的范围、章节依据、权威来源和代码验证状态
environment: 在线资料调研；2026-11-12
date: 2026-11-12
---
# 第一章调研与确认记录

## 范围确认

- 初始路线：`dx:cpp-learning/knowledge/roadmap.md` 第一阶段。
- 用户确认范围：完整第一阶段。
- 用户确认深度：实现级深入。
- 用户确认环境：跨平台 CMake，C++17。
- 用户确认结构：第一章拆成上下两课。
- 贯穿案例：游戏资源管理与弱引用缓存。

## 调研要点

1. 主流教程通常将资源管理组织为 RAII → 移动语义 → 智能指针的连续路径，而不是孤立介绍语法。
2. C++17 现代语法需要同时讲适用场景和生命周期边界，尤其是非拥有的 `string_view`。
3. 实现级内容需要补充 Rule of Zero/Five、移后对象、复制消除、控制块、弱引用和 `noexcept`。
4. 所有权 API 应区分拥有与非拥有参数，避免把所有接口都改成智能指针。
5. 性能结论必须来自测量；拷贝与移动示例只报告当前环境，不承诺固定加速比。

## 来源与核实标记

| 来源 | 用途 | 核实状态 |
|---|---|---|
| [cppreference：C++17](https://en.cppreference.com/cpp/17) | 确认 C++17 特性范围 | 已访问，2026-11-12 |
| [cppreference：结构化绑定](https://en.cppreference.com/cpp/language/structured_binding) | 绑定对象、数组与 tuple-like 类型的语义 | 已访问，2026-11-12 |
| [cppreference：`std::optional`](https://en.cppreference.com/cpp/utility/optional) | 可选值的所有权、访问与空状态 | 已检索，2026-11-12 |
| [cppreference：`std::variant`](https://en.cppreference.com/cpp/utility/variant) | 类型安全联合体与访问方式 | 已访问，2026-11-12 |
| [cppreference：`std::string_view`](https://en.cppreference.com/cpp/string/basic_string_view) | 非拥有字符序列视图及失效规则 | 已访问，2026-11-12 |
| [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) | RAII、Rule of Zero/Five、所有权参数设计 | 已检索，2026-11-12 |
| [Microsoft Learn：智能指针](https://learn.microsoft.com/en-us/cpp/cpp/smart-pointers-modern-cpp?view=msvc-170) | 现代 C++ 资源管理与智能指针用法 | 已检索，2026-11-12 |
| [Microsoft Learn：移动构造与移动赋值](https://learn.microsoft.com/en-us/cpp/cpp/move-constructors-and-move-assignment-operators-cpp?view=msvc-170) | 资源转移、自移动赋值与移后状态 | 已访问，2026-11-12 |
| [cppreference：`std::move_if_noexcept`](https://en.cppreference.com/cpp/utility/move_if_noexcept) | 容器重分配与强异常保证 | 已检索，2026-11-12 |
| [cppreference：`std::shared_ptr`](https://en.cppreference.com/cpp/memory/shared_ptr) | 共享所有权与控制块概念 | 已检索，2026-11-12 |
| [cppreference：`std::make_shared`](https://en.cppreference.com/cpp/memory/shared_ptr/make_shared) | 对象与控制块分配 | 已检索，2026-11-12 |
| [cppreference：`std::weak_ptr`](https://en.cppreference.com/cpp/memory/weak_ptr) | 临时所有权与缓存场景 | 已检索，2026-11-12 |
| [LearnCpp：智能指针与移动语义](https://www.learncpp.com/cpp-tutorial/introduction-to-smart-pointers-move-semantics/) | 主流教程章节组织方式 | 已检索，2026-11-12 |
| [LearnCpp：`weak_ptr` 与循环依赖](https://www.learncpp.com/cpp-tutorial/circular-dependency-issues-with-stdshared_ptr-and-stdweak_ptr/) | 常见循环所有权问题 | 已检索，2026-11-12 |

## 验证状态

- 2026-11-12：使用 Windows、CMake 4.2.1、MSVC 19.50 完成配置和构建。
- `ch01_modern_syntax`、`ch01_resource_manager`、`ch01_copy_move_benchmark` 均通过 CTest。
- 尝试使用 Clang 18.1.8 + Ninja 复验，但当前 Visual Studio STL 报告 `STL1000`，要求 Clang 19 或更高版本，因此未完成该工具链验证。
- GCC 与其他操作系统未在当前环境验证；正文不声称已覆盖这些工具链。
