---
title: RAII 专题调研与确认记录
series: cpp17-raii
topic: RAII 专题的范围、权威来源和代码验证
audience: 教程作者与审校者
outcome: 追溯专题结论、用户选择和实际测试范围
environment: 在线资料调研；Windows + MSVC 19.50；2026-11-12
date: 2026-11-12
---
# RAII 专题调研与确认记录

## 范围确认

- 用户要求：现有教程中的 RAII 需要深入。
- 初步建议：在第一章扩展异常安全、构造失败和析构约束。
- 用户选择：改为独立 RAII 专题。
- 最终结构：6 章模式 A；原第一章保留简介并链接本专题。
- 示例基线：C++17、跨平台 CMake。

## 调研要点

1. RAII 的核心是资源生命周期与对象生命周期绑定，并将资源可用性建立为类不变量。
2. 构造函数抛出时，未完整构造的对象不会执行自身析构；已完整构造的成员和基类会被销毁。
3. 成员按声明顺序构造、逆序销毁，不能依赖初始化列表的书写顺序改变这一点。
4. 析构函数应不抛异常；可能失败的关闭操作需要提供显式接口报告错误。
5. RAII 不仅保证释放资源，还为基本和强异常安全提供组合基础。
6. 资源包装器必须明确复制、移动或禁止复制，避免隐式浅拷贝导致双重释放。

## 权威来源

| 来源 | 用途 | 状态 |
|---|---|---|
| [cppreference：RAII](https://en.cppreference.com/cpp/language/raii) | 类不变量、标准库资源包装器 | 已访问，2026-11-12 |
| [cppreference：析构函数](https://en.cppreference.com/cpp/language/destructor) | 析构调用时机、构造失败与销毁顺序 | 已访问，2026-11-12 |
| [cppreference：对象生命周期](https://en.cppreference.com/cpp/language/lifetime) | 生命周期开始和结束 | 已检索，2026-11-12 |
| [cppreference：抛出异常](https://en.cppreference.com/cpp/language/throw) | 栈展开与异常对象 | 已检索，2026-11-12 |
| [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) | 资源管理、析构不失败、Rule of Zero/Five | 已检索，2026-11-12 |
| [Microsoft Learn：对象生命周期与 RAII](https://learn.microsoft.com/en-us/cpp/cpp/object-lifetime-and-resource-management-modern-cpp?view=msvc-170) | 自动对象与资源管理 | 已检索，2026-11-12 |
| [Microsoft Learn：异常安全设计](https://learn.microsoft.com/en-us/cpp/cpp/how-to-design-for-exception-safety?view=msvc-170) | 基本、强和不抛保证 | 已访问，2026-11-12 |
| [Microsoft Learn：异常处理实践](https://learn.microsoft.com/en-us/cpp/cpp/errors-and-exception-handling-modern-cpp?view=msvc-170) | 异常传播与栈展开 | 已检索，2026-11-12 |
| [Microsoft Learn：智能指针](https://learn.microsoft.com/en-us/cpp/cpp/smart-pointers-modern-cpp?view=msvc-170) | 标准库所有权包装器 | 已检索，2026-11-12 |

## 验证记录

- 2026-11-12：Windows、CMake 4.2.1、MSVC 19.50。
- 6 个示例均完成 Debug 构建，CTest 6/6 通过。
- 构造失败示例验证事件序列为“首成员构造 → 首成员析构”，外层析构未执行。
- 强异常保证示例在临时副本已修改后抛出异常，验证原状态保持不变。
- GCC、Clang 和其他操作系统未实际验证。
