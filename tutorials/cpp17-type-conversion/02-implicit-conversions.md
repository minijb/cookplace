---
title: 02 隐式转换规则
series: cpp17-type-conversion
chapter: 02
topic: 整型提升、usual arithmetic conversions、窄化与符号坑的完整规则
audience: 被 `-1 < 1u`、`size_t` 比较和静默截断坑过的开发者
outcome: 能预判任意算术表达式的结果类型，并列举自己代码里的窄化与符号坑位置
environment: C++17；Windows + MSVC 19.50 已验证（见 README）；2026-11-19
date: 2026-11-19
---
# 02 隐式转换规则

适用环境：C++17；配套代码为 [`02_implicit.cpp`](code/02_implicit.cpp)。

## 1. 整型提升：小于 int 的先变 int

比 `int` 窄的整型（`bool`、`char`、`signed char`、`unsigned char`、`short`、`unsigned short`）参与运算前，先提升为 `int`（若 `int` 装不下其全部取值则提升为 `unsigned int`）。提升是**值保持**的，这是转换阶梯里最安全的一级。

```cpp
unsigned char uc = 250;
auto sum = uc + uc;   // 两个 uc 先各自提升为 int 再相加
// decltype(sum) 是 int，值 500，不会在 unsigned char 里回绕
```

存在提升的原因：CPU 对「机器字长」类型的运算最快，语言直接规定小类型先归一化到 `int`。这也是 `char` 相加得到 `int`、`sizeof('a') == sizeof(int)` 一类「反直觉」现象的根源。

## 2. usual arithmetic conversions：同秩时 unsigned 赢

两个操作数类型不同时，提升之后继续归一化，规则按序：

1. 先各自整型提升；
2. 同类型 → 结束；
3. 同符号性 → 秩低的转成秩高的；
4. 符号性不同：
   - 无符号一方秩 ≥ 有符号一方 → **全体转成无符号**；
   - 有符号一方能装下无符号一方的全部取值（如 64 位 `long long` 对 32 位 `unsigned int`）→ 转成有符号；
   - 否则 → 转成无符号一方的对应有符号类型。

> ⚠️ 第 4.1 条是经典事故源：`-1 < 1u` 的结果**是 `false`**。同秩（都是 `int`/`unsigned int`）比较，`-1` 被转成 `0xFFFFFFFF`。`-1` 从此不再是 -1。

配套代码实测输出：

```text
-1 < 1u 结果 = false（-1 被转成 4294967295）
unsigned 0u - 1 = 4294967295（模 2^32 回绕）
```

无符号运算是模 2^N 运算：`0u - 1` 合法且结果为 `UINT_MAX`。逻辑上「减到负数」的计数器绝不能用无符号类型。

## 3. 窄化转换：拷贝初始化放行，列表初始化拦下

目标类型装不下源类型全部值就是窄化：浮点→整型（向零截断）、大整型→小整型、`double`→`float` 等。语言给了两道不同的门：

```cpp
double d = 3.9;
int i = d;      // 拷贝初始化：放行，静默截断为 3（MSVC /W4 给 C4244 警告）
int j{d};       // 列表初始化：编译错误！C++11 起花括号禁止窄化
```

实践建议：**可能窄化的地方全用花括号或显式 `static_cast`**。前者让编译器拦，后者让读者看见。

## 4. 浮点 ↔ 整型：两个方向的坑

- 浮点→整型：截断向零；若截断后的值超出整型表示范围，是**未定义行为**（如 `1e30` 转 `int`）。
- 整型→浮点：值超范围同样 UB；范围内也可能**丢精度**——`float` 尾数只有 24 位：

```text
16777217 转 float 再回来 = 16777216（精度丢失）
```

`16777217`（2^24 + 1）转 `float` 后变成 `16777216`。需要精确整数值穿过浮点接口时，用 `double` 或干脆别走浮点。

## 5. 指针转换：退化、上行与 void*

| 转换 | 写法 | 说明 |
|---|---|---|
| 数组→指针 | 隐式（退化） | `arr` 变成 `&arr[0]`，`sizeof`/`decltype` 语境例外 |
| 函数→指针 | 隐式（退化） | 函数名即地址 |
| 派生*→基* | 隐式（上行转换） | 安全，可能调整指针偏移 |
| 任意对象指针→`void*` | 隐式 | 丢掉类型信息 |
| `void*`→`T*` | **必须 `static_cast`** | 往返的唯一出口 |

`void*` 往返是唯一合法的「擦除再恢复」：存进去是 `int*`，取出来就必须 `static_cast<int*>`，取成别的类型同样是违反别名的 UB。

## 6. 头号日常坑：size_t 与 int 比较

`std::vector::size()` 返回无符号的 `size_t`。拿有符号 `int` 跟它比较，`int` 会被转成无符号数：

```text
直接写 n < vec.size() 得到 false（-1 被转成巨大无符号数）
```

`n = -1` 时，`n < vec.size()` 是 `false`——哪怕容器是空的。「循环到 `size()`」没问题，但「先减后比」「可能为负的下标」必须在比较前统一类型：

```cpp
if (n < static_cast<int>(vec.size()))   // 明确把 size 转回有符号再比
```

## 7. bool 转换：零与空即 false

整型值 0、空指针、（用户类型经 `explicit operator bool`）在布尔语境都转成 `false`，其余为 `true`。指针→`bool` 是语义转换（阶梯第 3 级）：`if (ptr)` 是惯用法，但把指针赋给 `bool` 变量就掩盖了意图，尽量少写。

```text
nullptr -> bool = false，&sum -> bool = true
```

## 本章要点

- 小类型先提升为 `int`，提升保值。
- 同秩混合符号运算，全体转无符号：`-1 < 1u == false`、`0u - 1 == UINT_MAX`。
- 窄化用花括号拦或 `static_cast` 标注，别让它静默发生。
- 浮点↔整型双向都可能丢值，超范围转整型是 UB。
- `size_t` 比较前先统一符号性；`void*` 往返必须 `static_cast`。

## 待深入

- 浮点环境与舍入模式对浮点转换的影响（`<cfenv>`）。
- `char8_t`（C++20）加入后的字符类型提升规则。
