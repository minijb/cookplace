---
title: 04 命名 cast 四剑客与 C 风格 cast
series: cpp17-type-conversion
chapter: 04
topic: static_cast、dynamic_cast、const_cast、reinterpret_cast 的语义边界，以及 C 风格 cast 为何禁用
audience: 知道四个 cast 名字但每次都靠猜的开发者
outcome: 拿到任何转换需求都能立刻选对 cast，并说清 C 风格 cast 的具体危害
environment: C++17；Windows + MSVC 19.50 已验证（见 README）；2026-11-19
date: 2026-11-19
---
# 04 命名 cast 四剑客与 C 风格 cast

适用环境：C++17；配套代码为 [`04_named_casts.cpp`](code/04_named_casts.cpp)。

## 1. 一张分工表

四个命名 cast 按「语义」分工，而不是按「类型」分工。选 cast 时先问「我要的语义是什么」：

| cast | 语义 | 典型场景 | 失败行为 |
|---|---|---|---|
| `static_cast` | 编译期明确语义的转换 | 枚举↔整数、浮点↔整数、上行转换、`void*` 往返、调用 `explicit` 构造 | 编译错误 |
| `dynamic_cast` | 运行期检查的多态下行/跨类转换 | 基类指针/引用 → 派生 | 指针返 `nullptr`；引用抛 `std::bad_cast` |
| `const_cast` | 增删 `const`/`volatile` | 适配只读旧 C 接口、const/非 const 重载去重 | 编译错误（类型不匹配时） |
| `reinterpret_cast` | 位模式重新解释 | 指针↔整数、按字节查看内存 | 几乎没有保证 |

> ⚠️ 四剑客里只有 `dynamic_cast` 会告诉你「转换失败」，其余三者的安全性完全依赖你对语义的判断。

## 2. static_cast：默认首选

语义明确、编译期完成的转换都用它：

```cpp
enum class GameState : int { Menu = 0, Playing = 1 };
int raw = static_cast<int>(GameState::Playing);      // 枚举 -> 底层整数
auto st = static_cast<GameState>(raw);               // 底层整数 -> 枚举
int truncated = static_cast<int>(3.99);              // 浮点 -> 整型，明确声明接受截断
```

下行转换（基→派生）它也做，但**没有任何检查**：

```cpp
Base* bp = &der;
Derived* dp = static_cast<Derived*>(bp);   // 编译器信任你

Base other;
// Derived* wrong = static_cast<Derived*>(&other);   // 编译能过！
// wrong->d;                                         // 运行期读越界内存：UB
```

`other` 里根本没有 `Derived` 的派生成员，`wrong->d` 读的是未分配的内存。**静态下行转换只用于「类型已被其他逻辑证明」的场景**；证明不了就用 `dynamic_cast`（第 5 章）。

## 3. dynamic_cast：会检查的那一个

```cpp
if (auto* ok = dynamic_cast<Derived*>(maybe_derived)) { /* 确实是 Derived */ }
auto* failed = dynamic_cast<Derived*>(really_base);
// failed == nullptr：类型不符，指针形式安全失败
```

它要求源类型多态（至少一个虚函数），运行期沿 RTTI 元数据走继承图。深入实现与成本见第 5 章。

## 4. const_cast：窄门，别当捷径

唯一能去 `const` 的工具。合法用途非常窄：

1. **适配只读旧接口**：C 接口参数没写 `const`，但契约保证只读：

```cpp
std::size_t legacy_string_length(char* s);           // 只读，但签名没 const
const char* msg = "hello legacy api";
auto len = legacy_string_length(const_cast<char*>(msg));  // 合法
```

2. **const/非 const 成员函数重载去重**：非 const 版本调 const 版本后去 const。

> ⚠️ 通过 `const_cast` 修改一个**本来就定义为 const** 的对象是 UB。编译器可能把真正的 const 对象放进只读段，写它直接崩溃。`const_cast` 去掉 const 后「改」之前，必须确认底层对象自身不是 const。

## 5. reinterpret_cast：低权限通行证

它基本不生成指令（指针↔整数除外），只是让编译器「换个类型名看这块内存」。给出的保证极少：

```cpp
int value = 42;
auto addr = reinterpret_cast<std::uintptr_t>(&value);  // 指针 -> 整数：合法
int* same = reinterpret_cast<int*>(addr);              // 整数 -> 指针：往返成立
// float f = 1.0f;
// int bits = *reinterpret_cast<int*>(&f);             // 违反严格别名：UB！
```

最后一行是高频错误：通过不相关类型的左值访问对象违反严格别名规则（第 6 章给正解）。`reinterpret_cast` 的合法高频用途只有：指针↔整数往返（如哈希指针）、`std::byte`/`unsigned char*` 视角看内存、以及实现层的存储管理。

## 6. C 风格 cast：为什么全项目禁用

`(T)x` 不是某一种 cast，而是编译器的**按序尝试**：先试 `const_cast`，再试带扩展的 `static_cast`，再试 `static_cast` + `const_cast` 组合，再试带扩展的 `reinterpret_cast`，最后 `reinterpret_cast` + `const_cast`。第一个能编译的胜出。

两个具体危害：

**危害一：静默升级成最危险的 cast。**

```cpp
struct Unrelated { int u = 1; };
Unrelated* ur = (Unrelated*)&value;      // C 风格：编译照过（变身 reinterpret_cast）
// static_cast<Unrelated*>(&value);      // 命名 cast：编译报错，拦住你
```

**危害二：无视访问控制。**

```cpp
class PrivBase { public: int x = 10; };
class PrivDerived : private PrivBase { /* ... */ };

PrivDerived pd;
// static_cast<PrivBase*>(&pd);    // 编译错误：继承是 private，外部不得上行
PrivBase* pb = (PrivBase*)&pd;     // C 风格：静默无视 private 继承，编译通过
```

命名 cast 无法表达的转换，C 风格 cast 全都能「办成」，而且从代码上完全看不出办的是哪一种。另外命名 cast 全文检索 `_cast` 即可审计，C 风格只能靠正则猜。**Core Guidelines（`T.4`、`C.146` 等）与主流编码规范一致要求禁用 C 风格 cast。**

## 本章要点

- 选 cast 问语义：编译期明确 → `static_cast`；运行期验型 → `dynamic_cast`；去 const → `const_cast`（仅适配层）；位重解释 → `reinterpret_cast`（仅指针↔整数/字节视角）。
- `static_cast` 下行转换无检查，类型错了是 UB。
- `const_cast` 只为适配旧接口，不许借此改真 const 对象。
- C 风格 cast = 按序尝试的万能钥匙：无视 private 继承、静默升级 reinterpret——禁用。

## 待深入

- 函数式转换 `T(x)` 与 `T{x}` 的差异（后者禁止窄化，前者等同 C 风格）。
- `dynamic_cast` 在 C 风格 cast 尝试序列中的位置（不在序列里——C 风格 cast 永远不做运行期检查）。
