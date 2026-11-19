---
title: 03 用户自定义转换与 explicit
series: cpp17-type-conversion
chapter: 03
topic: 转换构造函数、转换运算符、二段式转换序列与 explicit 的设计决策
audience: 设计自己的类接口，想知道「要不要写 explicit」「为什么两跳转换不行」的开发者
outcome: 能为自己的类型正确配置隐式/显式转换面，并解释每条规则的依据
environment: C++17；Windows + MSVC 19.50 已验证（见 README）；2026-11-19
date: 2026-11-19
---
# 03 用户自定义转换与 explicit

适用环境：C++17；配套代码为 [`03_user_defined.cpp`](code/03_user_defined.cpp)。

## 1. 两个方向，两个构件

用户自定义类型参与转换的构件有两个，方向相反：

| 构件 | 方向 | 写法 |
|---|---|---|
| 转换构造函数 | 别的类型 → 本类型 | `Seconds(double s)` 单参数构造 |
| 转换运算符 | 本类型 → 别的类型 | `operator double() const` |

```cpp
struct Seconds {
    Seconds(double s) : value(s) {}    // double 可以静默变成 Seconds
    double value;
};

class Ratio {
public:
    operator double() const { return ratio; }              // Ratio 可以静默变成 double
private:
    double ratio;
};
```

两者任何一个不写 `explicit`，就给隐式路线开了一扇门：函数实参、初始化、`return` 处都可能静默触发。

## 2. 隐式转换序列的三段式

编译器把一个实参适配成形参类型时，构造的序列固定为三段：

```mermaid
flowchart LR
  S[标准转换序列<br/>提升/算术/指针] --> U[用户自定义转换<br/>构造函数 或 运算符<br/>至多一个]
  U --> T[标准转换序列<br/>再补一段]
```

关键约束：**每条隐式转换链最多一个用户自定义转换**。所以 `A→B` 和 `B→C` 都各自合法，但 `A→C` 的两跳链不合法：

```cpp
struct A { A(int) {} };
struct B { B(const A&) {} operator A() const { return A{0}; } };
struct C { C(const B&) {} };

void takes_c(const C&) {}
B b = A{1};          // OK：A -> B，一个用户转换
takes_c(C{b});       // OK：B -> C，一个用户转换
// takes_c(A{1});    // 编译错误：A -> B -> C 需要两个用户转换
```

这条限制让「转换传染病」止步于一层——但也只止步一层，单层隐式转换已经足够制造 bug。

## 3. explicit：构造函数侧的闸门

`explicit` 让转换构造函数只能走显式路线：

```cpp
struct PacketId {
    explicit PacketId(int id) : value(id) {}
    int value;
};

PacketId p(7);                          // 直接初始化：OK
PacketId p2 = static_cast<PacketId>(8); // 显式转换：OK
// PacketId bad = 9;                    // 编译错误
// void f(PacketId); f(9);              // 编译错误：隐式通道被封
```

用标准工具验证这两个性质，可以写进单测：

```cpp
static_assert(std::is_constructible<PacketId, int>::value, "显式构造可构");
static_assert(!std::is_convertible<int, PacketId>::value, "explicit 禁止隐式转换");
```

> ⚠️ 拷贝列表初始化 `PacketId p3 = {9}` 对 `explicit` 构造函数同样是编译错误——花括号拷贝初始化不豁免 explicit。

**默认写 explicit，确认需要隐式转换再放开**，是现代 C++ 的共识。MISRA C++:2023 Rule 15.1.3 与 AUTOSAR A12-1-2 都把它定为强制规则。判断标准（foonathan 的设计准则）：只有**宽化且无歧义**的转换才值得隐式——比如 `std::string` 从字符串字面量隐式构造是便利性与风险的良好折中；`Seconds` 从裸 `double` 隐式构造则是把「秒」和「毫秒」「米」混在一起的隐患。

## 4. explicit operator bool：只开布尔一扇门

不加 `explicit` 的 `operator bool()` 会引发灾难链：对象能隐式变 `bool`，`bool` 又能隐式变 `int`，于是 `obj << 1`、`obj + 42` 全部合法。C++11 的解法是 `explicit operator bool()`：

```cpp
class Ratio {
public:
    explicit operator bool() const { return ratio > 0.0; }
};

if (half) { ... }                    // 布尔语境：OK（语境转换豁免 explicit）
bool ok = static_cast<bool>(half);   // 显式转换：OK
// bool bad = half;                  // 拷贝初始化：编译错误
// int n = half;                     // 想当整数用：编译错误
```

「语境转换」（contextual conversion）在 `if`、`while`、`!`、`&&`、`||`、`?:` 里豁免 `explicit`——这就是 `explicit operator bool` 既好用又安全的原因：用的时候只出现在真布尔语境，永远污染不了算术。**这是唯一值得写的非显式方向转换运算符，其他转换运算符一律加 `explicit`**（AUTOSAR A13-5-2）。

## 5. 决策表

| 场景 | 写法 | 理由 |
|---|---|---|
| 单参数构造，语义是「从 X 造出我」 | `explicit` | 防止无关类型静默变成本类型 |
| 字符串字面量 → `string_view`/`string` | 保持隐式 | 宽化、无歧义、高频便利 |
| 转换运算符到 `bool` | `explicit operator bool` | 布尔语境豁免，算术语境封死 |
| 其他转换运算符 | `explicit` 或干脆删掉 | 隐式对象转换几乎总是 bug 源 |
| 数值量纲类型（秒/米/像素） | `explicit` + 命名工厂 | `Seconds::from_ms()` 比隐式 `double` 安全 |

## 本章要点

- 两个构件两个方向：转换构造函数进、转换运算符出。
- 隐式链 = 标准 + 至多一个用户转换 + 标准；两跳直接编译错误。
- 单参数构造默认 `explicit`；`is_convertible` 负值进单测。
- `explicit operator bool` 是唯一推荐的隐式友好转换运算符。

## 待深入

- 重载决议中隐式转换序列的分级（exact match / promotion / conversion）与二义性。
- `if constexpr` 配合 `explicit` 做「按条件转换面」（C++20 concepts 视角）。
