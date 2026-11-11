---
title: 语言核心深入
brief: 现代 C++（至 C++17）核心语言特性：RAII、移动语义、模板、lambda、constexpr
tags: [cpp, core]
---
# 语言核心深入

基线 C++17。只记已确认的结论，按主题分节积累；结构遵循 `dx:cpp-learning/spec/note-template.md`。

## 资源与所有权
- RAII：资源生命周期绑定对象生命周期，析构函数是唯一可靠的清理点。
- Rule of Zero / Rule of Five：优先 Zero（成员用智能指针/容器托管资源）；一旦自定义析构/拷贝/移动之一，五个特殊成员都要整体考虑。
- 智能指针：`unique_ptr` 独占且近零开销；`shared_ptr` 引用计数（控制块有原子操作成本，循环引用用 `weak_ptr` 破）。

## 移动语义与值类别
- 值类别：lvalue / prvalue / xvalue；移动构造把资源「偷」过来，被移对象置于有效但未指定状态。
- `std::move` 只是无条件转 rvalue 的 cast，本身不移动任何东西；完美转发用 `std::forward` 配合转发引用 `T&&`。

## 现代语法（C++11–17）
- auto、结构化绑定、`if constexpr`、`std::optional` / `std::variant` / `std::any`、`std::string_view`（不拥有数据，警惕悬垂）、inline 变量。
- lambda：值/引用捕获语义、初始化捕获；闭包类型匿名，无捕获可转函数指针。

## 模板与泛型
- 函数模板/类模板与实参推导；SFINAE 在 C++17 常可用 `if constexpr` 替代，可读性更好。
- `constexpr` 函数 C++17 起适用面扩大，可同时服务编译期与运行时。

## 待深入
- concepts / 协程 / 三路比较 `<=>`（均为 C++20，本 dx 仅作展望）
- `noexcept` 与移动安全、ADL 与名字查找
