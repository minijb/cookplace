---
title: 类的内存结构（对象布局）
brief: 对象布局规则：对齐与 padding、vptr/虚表、多继承与虚继承布局、RTTI 成本与观测工具
tags: [cpp, memory, object-model]
---
# 类的内存结构（对象布局）

基线 C++17，ABI 以 Itanium（GCC/Clang）为主要参照、MSVC 差异标注。只记已确认结论，出处见文末参考。

## 基本规则
- 只有**非静态数据成员**占对象空间；成员函数、静态成员都在对象外。
- 同一访问控制段内，成员按声明顺序排列；不同访问段之间的相对顺序**未由标准规定**。
- 只有 standard-layout 类型的布局是可移植确定的；跨 ABI 边界传布局敏感对象时按此收紧。

## 对齐与 padding
- 类的对齐值 = 所有成员与基类对齐值的最大者；`sizeof` 向上取整到对齐倍数。
- padding 出现在成员之间（满足各自对齐）与对象尾部（满足整体对齐）。
- 成员按「从大到小」排序可减少 padding。
- 空类 `sizeof == 1`（保证对象地址唯一）；空基类优化（EBO）让空基类不占空间。

## 虚函数布局
- 类一旦有虚函数：每个对象头部多一个 vptr（x64 上 8B，通常在偏移 0）；每个类一张虚表，位于只读段。
- 虚表槽位是虚函数指针；Itanium ABI 中虚表还含 offset-to-top 与 typeinfo 指针（RTTI 的数据来源）。
- 虚调用路径：取 vptr → 查槽 → 间接调用；成本 = 两次内存访问 + 阻碍内联，热路径小函数慎用虚。
- 成员函数（含虚函数本体）不占对象空间，占的是类级虚表空间。

## 继承布局
- 单继承：基类子对象在前，派生成员在后；单 vptr 复用基类的。
- 多继承：每个**含虚函数**的基类子对象各带一个 vptr；跨基类调用插入 this 指针调整（thunk）。
- 虚继承：虚基子对象共享一份，通过虚表中的偏移槽定位，访问虚基成员多一次间接。

## RTTI 与转换成本
- `dynamic_cast` / `typeid` 走虚表中的 typeinfo；`-fno-rtti` 下不可用。
- trivially copyable + standard-layout 才可安全 `memcpy`；序列化/跨语言边界按此判定。

## 观测工具
- `sizeof` / `alignof` / `offsetof`；clang `-Xclang -fdump-record-layouts`、GCC `-fdump-class-hierarchy`、pahole 看真实布局。

## 待深入
- `[[no_unique_address]]`（C++20）、MSVC 与 Itanium ABI 布局差异、成员函数指针的表示
