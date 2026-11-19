---
title: 类型转换专题调研与确认记录
series: cpp17-type-conversion
topic: 类型转换专题的范围、权威来源和代码验证
audience: 教程作者与审校者
outcome: 追溯专题结论、用户选择和实际测试范围
environment: 在线资料调研；Windows + MSVC 19.50；2026-11-19
date: 2026-11-19
---
# 类型转换专题调研与确认记录

## 范围确认

- 用户要求：深入学习 C++ 类型转换（`dx:cpp-learning` 下原本无专门知识文件）。
- 调研后建议章节并二次确认：用户选择「全部 9 章完整学习」。
- 落库形式（用户确认）：分章教程 + 结论按 `dx:cpp-learning/spec/note-template.md` 回写 knowledge；每章最小可运行示例。
- 结构调整：C 风格 cast 并入命名 cast 章（第 4 章），9 章压缩为 8 章以符合目录结构规范的模式 A（章节 ≤ 8）。
- 示例基线：C++17、跨平台 CMake；第 7 章无 UE 环境，采用语义模拟并明示。

## 调研要点

1. 主流教程章节结构高度一致（learncpp 10.x、C++ Primer 4.11、cppreference [conv]/[expr.cast]）：隐式转换（提升/算术转换/窄化）→ 用户自定义转换与 explicit → 命名 cast → C 风格 cast 尝试顺序。
2. `-1 < 1u` 为 false、`size_t` 与 int 比较、浮点截断是社区最高频的三类符号/窄化坑。
3. implicit conversion sequence 三段式：标准 → 至多一个用户自定义转换 → 标准；两跳用户转换病构。
4. `explicit operator bool` 的语境转换豁免是唯一推荐的隐式友好转换运算符；MISRA C++:2023 R15.1.3 与 AUTOSAR A13-5-2/A12-1-2 强制 explicit。
5. C 风格 cast 按 const_cast → static_cast（带扩展，含无视 private 继承）→ + const_cast → reinterpret_cast（带扩展）→ + const_cast 顺序尝试。
6. `dynamic_cast` 仅下行/跨类/`void*` 三种真动态；Itanium ABI `__dynamic_cast` 沿继承图搜索；libc++ 曾测得比 libstdc++ 慢 8–30 倍（后续优化修复）；指针失败返 nullptr、引用抛 bad_cast。
7. 严格别名规则与 union 非活动成员使 `reinterpret_cast` 解引用和 union 双关成为 UB；正解为 memcpy（C++17）与 `std::bit_cast`（C++20）；clang-tidy 提醒 bit_cast 不能机械替换指针伪转换。
8. UE 以 `/GR-` 关 RTTI；`Cast<T>` 走 UClass 反射 `IsChildOf`，非编辑器构建判型近常数、编辑器 O(继承深度)，失败返 nullptr；UObject 下行禁用 `static_cast`（官方论坛验证帖 TLDR）。

## 权威来源

| 来源 | 用途 | 状态 |
|---|---|---|
| [cppreference：Implicit conversions](https://en.cppreference.com/w/cpp/language/implicit_conversion) | 标准转换序列、提升、算术转换 | 已检索，2026-11-19 |
| [cppreference：Usual arithmetic conversions](https://en.cppreference.com/w/cpp/language/usual_arithmetic_conversions) | 混合算术归一化规则 | 已访问，2026-11-19 |
| [cppreference：Explicit type conversion](https://en.cppreference.com/w/cpp/language/explicit_cast) | 命名 cast 语义、C 风格尝试顺序 | 已访问，2026-11-19 |
| [cppreference：reinterpret_cast](https://en.cppreference.com/w/cpp/language/reinterpret_cast) | 类型别名与指针转换保证 | 已访问，2026-11-19 |
| [Microsoft Learn：Casting Operators](https://learn.microsoft.com/en-us/cpp/cpp/casting-operators?view=msvc-170) | 四命名 cast 总览 | 已检索，2026-11-19 |
| [Microsoft Learn：Type conversions and type safety](https://learn.microsoft.com/en-us/cpp/cpp/type-conversions-and-type-safety-modern-cpp?view=msvc-170) | 类型安全视角的转换分级 | 已检索，2026-11-19 |
| [Microsoft Learn：User-Defined Type Conversions](https://learn.microsoft.com/en-us/cpp/cpp/user-defined-type-conversions-cpp?view=msvc-170) | 转换构造/运算符、bool 陷阱 | 已检索，2026-11-19 |
| [C++ Primer 5e 4.11 Type Conversions](https://cpp-primer.pages.dev/book/049-4.11._type_conversions.html) | 教学结构参照 | 已检索，2026-11-19 |
| [LearnCpp 10.6 Explicit type conversion](https://www.learncpp.com/cpp-tutorial/explicit-type-conversion-casting-and-static-cast/) | 教学结构参照、命名 cast 安全性表 | 已检索，2026-11-19 |
| [StackOverflow：When should the four casts be used?](https://stackoverflow.com/questions/332030/when-should-static-cast-dynamic-cast-const-cast-and-reinterpret-cast-be-used) | 选型惯例 | 已检索，2026-11-19 |
| [foonathan：Guidelines for constructor and cast design](https://www.foonathan.net/2018/01/constructors-casts-design/) | implicit/explicit 设计准则 | 已检索，2026-11-19 |
| [MISRA C++:2023 Rule 15.1.3](https://www.mathworks.com/help/bugfinder/ref/misracpp2023rule15.1.3.html) | 单参构造/转换运算符须 explicit | 已检索，2026-11-19 |
| [AUTOSAR A13-5-2](https://www.mathworks.com/help/bugfinder/ref/autosarc14rulea1352.html) | 转换运算符须 explicit | 已检索，2026-11-19 |
| [Stroustrup：Fast Dynamic Casting](https://www.stroustrup.com/fast_dynamic_casting.pdf) | dynamic_cast 成本与失败 O(1) 方案 | 已检索，2026-11-19 |
| [LLVM Discourse：dynamic_cast in libcxxabi](https://discourse.llvm.org/t/performance-issues-of-dynamic-cast-in-libcxxabi/66296) | libc++/libstdc++ 实现性能差 | 已检索，2026-11-19 |
| [Itanium dynamic_cast from scratch（讲义）](https://itp.uni-frankfurt.de/~sciarra/C++_User_Group/Talks/2018_11.pdf) | 三种真动态 cast 与 ABI | 已检索，2026-11-19 |
| [tttapa：Don't use unions or pointer casts for type punning](https://tttapa.github.io/Pages/Programming/Cpp/Practices/type-punning.html) | 严格别名与双关正解 | 已检索，2026-11-19 |
| [P0476：Bit-casting object representations](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0476r1.html) | std::bit_cast 提案动机 | 已检索，2026-11-19 |
| [clang-tidy：bugprone-bitwise-pointer-cast](https://clang.llvm.org/extra/clang-tidy/checks/bugprone/bitwise-pointer-cast.html) | bit_cast 误用提醒 | 已检索，2026-11-19 |
| [UE 官方论坛：Cast<T> and why not static_cast](https://forums.unrealengine.com/t/unreal-c-cast-t-function-and-why-you-dont-want-to-use-static_cast-for-uobjects/131425) | Cast 语义、成本、失败行为 | 已检索，2026-11-19 |
| [Peter Leontev：How UE Cast<T> works](https://peterleontev.com/blog/unreal_cast/) | TCastImpl 源码解析 | 已检索，2026-11-19 |
| [UE 论坛：Enable RTTI](https://forums.unrealengine.com/t/compiler-enable-run-time-type-information/47062) | UE 关闭 RTTI 的原因 | 已检索，2026-11-19 |

## 验证记录

- 2026-11-19：Windows、CMake 4.2.1、MSVC 19.50（Visual Studio 生成器，`/W4 /permissive- /utf-8`）。
- 8 个示例均完成 Debug 构建且零警告，CTest 8/8 通过。
- 第 02 章：`-1 < 1u == false`、`0u - 1 == 4294967295`、`16777217 -> float -> 16777216` 实测复现。
- 第 05 章：多继承 `Entity*`/`Weapon*` 地址差 16 字节实测；1×10⁷ 次 dynamic_cast 289 ms 对比 static_cast 7.4 ms（Debug，量级参考）。
- 第 06 章：`1.0f` 位模式 `0x3F800000` 提取与无损往返实测。
- 第 08 章：五道练习断言全部通过（`0x3F800000` 往返、explicit 断言、dynamic_cast 空指针分支）。
- 第 07 章为语义模拟，未在 UE 环境验证；机制描述以官方论坛帖与源码解析交叉核实为准。
- GCC、Clang 和其他操作系统未实际验证。
