---
title: 04 拷贝、移动与所有权
series: cpp17-raii
chapter: 04
topic: 为资源包装器设计深拷贝、移动或禁止复制的完整语义
audience: 已掌握构造和析构，需要实现资源类的 C++ 开发者
outcome: 能应用 Rule of Zero/Five，避免双重释放并定义移后状态
environment: C++17；CMake 3.16+；Windows + MSVC 19.50 已验证；2026-11-12
date: 2026-11-12
---
# 04 拷贝、移动与所有权

适用环境：C++17；配套代码为 [`04_copy_move.cpp`](code/04_copy_move.cpp)。

## 1. 先决定资源能否复制

资源包装器有三种常见策略：

| 策略 | 含义 | 示例 |
|---|---|---|
| 深拷贝/复制底层资源 | 两个对象各自拥有独立资源 | 独立字节缓冲区 |
| 禁止复制，只允许移动 | 资源只能独占并转移 | 文件、锁、GPU 独占句柄 |
| 共享所有权 | 多个对象共同延长资源生命周期 | 确实需要共享的资源 |

不能默认复制指针成员就完成“复制资源”。那通常只是复制句柄值，并可能导致双重释放。

## 2. Rule of Zero 优先

若成员已经是 `string`、`vector`、`unique_ptr` 等正确资源类型，优先让外层类使用编译器生成的特殊成员。这样成员的拷贝、移动和销毁语义会自然组合。

只有直接管理资源或需要定制语义时，才完整审查析构、拷贝构造、拷贝赋值、移动构造和移动赋值，这就是 Rule of Five。

用户声明的析构、拷贝或移动操作会影响其他特殊成员的隐式生成。不要依赖模糊的“编译器应该还会生成”假设；需要时明确使用 `=default` 或 `=delete`。

## 3. 教学用 `Handle`：复制表示取得独立所有权

配套代码用存活计数模拟“复制句柄时取得一份独立资源”：

```cpp
Handle(const Handle& other) : id_(other.id_) {
    if (id_ >= 0) {
        ++live_count_;
    }
}
```

真实操作系统的句柄复制可能需要专门的 duplicate API；不能只增加计数或复制整数值。本例的计数仅用于让拷贝与释放路径可观察。

空句柄用 `id_ == -1` 表示，复制空句柄不会增加资源数。

## 4. 拷贝赋值使用 copy-and-swap

```cpp
Handle& operator=(const Handle& other) {
    Handle copy(other);
    swap(copy);
    return *this;
}
```

先在临时副本中完成可能失败的复制，再交换。若复制抛出异常，目标仍保持原状态；成功后临时对象释放目标旧资源。这也覆盖了自拷贝赋值。

## 5. 移动转移所有权

```cpp
Handle(Handle&& other) noexcept : id_(std::exchange(other.id_, -1)) {}

Handle& operator=(Handle&& other) noexcept {
    if (this != &other) {
        release();
        id_ = std::exchange(other.id_, -1);
    }
    return *this;
}
```

移动构造把源句柄置空；移动赋值先释放目标旧资源，再接管源资源。源对象仍可安全析构和重新赋值。

```cpp
Handle moved(std::move(copy));
assert(copy.id() == -1);
assert(moved.id() == 10);
assert(Handle::live_count() == 2);
```

这里 `-1` 是本类明确提供的移后状态。一般类型的移后对象只保证有效但未指定，不能都假设为空。

## 6. 释放逻辑只保留一份

```cpp
void release() noexcept {
    if (id_ >= 0) {
        --live_count_;
        id_ = -1;
    }
}
```

析构和移动赋值共用 `release()`，避免两处清理逻辑逐渐不一致。`release()` 幂等且不抛异常，使空句柄、重复清理和移后析构都安全。

## 7. 不抛移动与容器

```cpp
static_assert(std::is_nothrow_move_constructible_v<Handle>);
static_assert(std::is_nothrow_move_assignable_v<Handle>);
```

真实不抛出的移动操作应标记 `noexcept`。这不仅是性能提示，也会影响容器在需要强异常保证时选择移动还是复制已有元素。

## 8. 练习

1. 将 `Handle` 改成不可复制、只可移动，并删除不再需要的复制操作。
2. 为 `Handle` 添加显式 `valid()`，定义空句柄上的合法操作。
3. 测试拷贝赋值和移动赋值前后的存活计数。
4. 用 `unique_ptr` 重新实现独占包装器，比较手写代码量。

## 小结

资源类的特殊成员必须共同表达同一所有权模型。优先组合现有 RAII 类型；直接管理资源时，完整设计五个特殊成员。

## 参考资料

- [C++ Core Guidelines：Rule of Zero/Five](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [Microsoft Learn：移动构造与移动赋值](https://learn.microsoft.com/en-us/cpp/cpp/move-constructors-and-move-assignment-operators-cpp?view=msvc-170)
- [cppreference：移动构造函数](https://en.cppreference.com/cpp/language/move_constructor)
