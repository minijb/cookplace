---
title: 03 析构函数与异常
series: cpp17-raii
chapter: 03
topic: 编写不抛异常的析构函数，并处理显式关闭与多态销毁
audience: 已理解对象生命周期和构造失败的 C++ 开发者
outcome: 能选择正确的析构时机，避免栈展开期间终止，并设计可报告错误的关闭接口
environment: C++17；CMake 3.16+；Windows + MSVC 19.50 已验证；2026-11-12
date: 2026-11-12
---
# 03 析构函数与异常

适用环境：C++17；配套代码为 [`03_destructors.cpp`](code/03_destructors.cpp)。

## 1. 析构函数何时自动调用

常见触发点包括：

- 局部对象离开作用域。
- 临时对象在完整表达式结束时销毁，除非生命周期被合法延长。
- 动态对象被 `delete` 或由智能指针销毁。
- 异常栈展开销毁已完整构造的自动对象。
- 程序或线程结束阶段销毁静态、线程局部对象。

析构函数体执行后，非静态数据成员按声明逆序销毁，随后执行基类销毁过程。因此，析构函数体中仍能访问尚未销毁的成员。

## 2. 析构函数不应让异常逃出

析构函数通常默认是 `noexcept`。在异常栈展开期间再让析构函数抛出异常，会导致程序调用 `std::terminate`；即使没有另一个异常，让析构失败也会使调用方难以维持资源不变量。

C++ Core Guidelines 的原则是“析构不能失败”，并要求析构函数 `noexcept`。本课用类型特征验证：

```cpp
static_assert(std::is_nothrow_destructible_v<DeviceHandle>);
```

如果底层关闭操作可能失败，应区分两件事：

1. 析构函数执行不抛异常的兜底清理。
2. 需要知道关闭结果时，调用显式 `close()` 等接口取得错误。

## 3. 显式关闭与幂等清理

```cpp
class DeviceHandle {
public:
    explicit DeviceHandle(int id) : id_(id) {
        ++open_count_;
    }

    ~DeviceHandle() noexcept {
        static_cast<void>(close());
    }

    DeviceHandle(const DeviceHandle&) = delete;
    DeviceHandle& operator=(const DeviceHandle&) = delete;

    bool close() noexcept {
        if (id_ < 0) {
            return true;
        }
        id_ = -1;
        --open_count_;
        return true;
    }
```

`close()` 将句柄置为无效，因此重复调用不会重复释放；析构再次调用也安全。真实设备的 `close()` 可以返回错误，但析构不能依赖抛出异常来报告。

```cpp
DeviceHandle device(7);
assert(DeviceHandle::open_count() == 1);

const bool closed = device.close();
assert(closed);
assert(!device.is_open());
assert(DeviceHandle::open_count() == 0);

const bool closed_again = device.close();
assert(closed_again);
```

显式关闭适合需要立即释放或观察结果的场景；析构保证忘记显式关闭时仍有兜底路径。

> ⚠️ 如果关闭失败意味着资源状态未知，包装器必须定义失败后的状态和处理策略，不能简单假装已经释放。

## 4. 不要在析构中做高风险工作

析构函数应避免：

- 可能抛异常的分配、日志或回调。
- 等待另一个可能已停止的线程。
- 依赖已经进入销毁阶段的外部单例。
- 在对象所有权未厘清时触发重入回调。

复杂关闭协议通常应提供显式两阶段接口：`stop()`/`close()` 处理可失败步骤，析构只负责不可失败的最终兜底。

## 5. 多态销毁是另一项生命周期责任

若通过基类指针删除派生对象，基类通常需要 `virtual` 析构函数，否则可能只执行基类销毁逻辑并产生未定义行为。若基类不用于多态删除，则不应仅为“看起来通用”而引入虚析构。

智能指针可以封装自定义删除器，但删除器仍必须遵循不抛异常和资源语义明确的约束。

## 6. 练习

1. 为 `DeviceHandle` 增加移动操作，保证源对象析构不会关闭已转走句柄。
2. 模拟一次关闭失败，设计 `close()` 的返回类型和对象后置状态。
3. 解释为什么析构中使用 `std::uncaught_exceptions()` 不应成为普通关闭错误的处理机制。
4. 审查一个继承体系，确认是否存在通过基类指针删除派生对象的路径。

## 小结

析构函数是最后的清理保障，不适合承担可失败业务。让析构不抛异常，并为需要结果的关闭操作提供显式接口。

## 参考资料

- [cppreference：析构函数](https://en.cppreference.com/cpp/language/destructor)
- [C++ Core Guidelines：析构不能失败](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [Microsoft Learn：异常安全设计](https://learn.microsoft.com/en-us/cpp/cpp/how-to-design-for-exception-safety?view=msvc-170)
