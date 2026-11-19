---
title: 06 类型双关与严格别名
series: cpp17-type-conversion
chapter: 06
topic: 为什么 reinterpret_cast/union 双关是 UB，memcpy 与 bit_cast 才是正解
audience: 需要把对象按字节序列化、把位模式当整数看的开发者
outcome: 能写出不违反严格别名规则的位级转换，并识别存量代码里的 UB 双关
environment: C++17；Windows + MSVC 19.50 已验证（见 README）；2026-11-19
date: 2026-11-19
---
# 06 类型双关与严格别名

适用环境：C++17；配套代码为 [`06_type_punning.cpp`](code/06_type_punning.cpp)。本章与 `dx:cpp-learning/knowledge/object-layout.md` 的 memcpy 安全判定（trivially copyable + standard-layout）衔接。

## 1. 直觉写法是 UB

把 `float` 的位模式当整数看（序列化、哈希、网络传输的常见需求），直觉写法全都是 UB：

```cpp
float f = 1.0f;
// int bits_bad = *reinterpret_cast<int*>(&f);   // UB：违反严格别名
// union { float fv; int iv; } u; u.fv = f;
// int bits_union = u.iv;                         // C++ 中读非活动成员是 UB
```

两条规则分别击中它们：

- **严格别名规则**：允许编译器假定「不同类型的左值不会指向同一内存」（`char`/`unsigned char`/`std::byte` 豁免）。通过 `int*` 去读一个 `float` 对象，编译器可以假定这两次访问互不相关，进而重排或删除你的读取。
- **union 非活动成员**：C++ 规定只读最后写入的成员（活动成员）。写入 `fv` 再读 `iv` 在 C++ 里没有定义（C 语言后来允许了，C++ 至今没有）。

这类代码「碰巧能跑」是因为没开优化的编译器不做别名分析——`-O2` 之后被重排的案例大量存在。

## 2. 正解一：memcpy 按字节复制

`memcpy` 复制的是**对象表示**（字节），不经过任何类型的左值访问，因此不违反别名规则：

```cpp
template <class Dst, class Src>
Dst bit_cast_copy(const Src& src) {
    static_assert(sizeof(Dst) == sizeof(Src), "bit 级转换要求大小一致");
    static_assert(std::is_trivially_copyable<Src>::value, "Src 必须平凡可复制");
    static_assert(std::is_trivially_copyable<Dst>::value, "Dst 必须平凡可复制");
    Dst dst;
    std::memcpy(&dst, &src, sizeof(Dst));
    return dst;
}

float f = 1.0f;
auto bits = bit_cast_copy<std::uint32_t>(f);   // 0x3F800000
```

实测：

```text
float 1.0000 的位模式 = 0x3F800000（IEEE 754 符号 0，指数 127，尾数全 0）
位模式复制回 float = 1.0000（无损往返）
```

约束是 `trivially copyable`：有虚表、有引用成员、有非平凡构造/析构的类型，其字节序列不构成合法的对象表示，双关结果未定义。这个判定与 `dx:cpp-learning/knowledge/object-layout.md` 的序列化规则一致。

## 3. 正解二：字节类型视角

严格别名规则**明文豁免** `char`、`unsigned char`、`std::byte`：任何对象都可以通过字节类型左值查看。所以「看内存长什么样」永远合法：

```cpp
std::uint32_t value = 0x12345678;
auto* bytes = reinterpret_cast<const unsigned char*>(&value);
// 78 56 34 12（x86 小端序：低字节在前）
```

注意方向：字节视角看强类型，合法；强类型视角看字节缓冲（把字节缓冲直接 `reinterpret_cast` 成 `Config*` 再解引用），违反别名且要处理对齐——要读值就用 `memcpy` 取出。

## 4. C++20 展望：std::bit_cast

本 dx 基线 C++17，但 `std::bit_cast`（C++20，`<bit>`）值得现在了解，因为它是上述 memcpy 助手的标准答案：

```cpp
// #include <bit>   // C++20
// auto bits20 = std::bit_cast<std::uint32_t>(f);   // constexpr，编译期可用
```

比 memcpy 助手的优势：`constexpr`（编译期求值）、无 `aligned_storage` 样板、语义由标准背书。迁移成本为零——把模板调用换成 `bit_cast` 即可。

> ⚠️ clang-tidy 的 `bugprone-bitwise-pointer-cast` 检查提醒：不要把 `*reinterpret_cast<T*>(&x)` 机械替换成 `*std::bit_cast<T*>(&x)`——对**指针**做位转换两个都一样是错的。`bit_cast` 适用于值级转换（float↔uint32_t、POD↔字节缓冲），不适用于「把 int 当指针解引用」这类指针伪转换。

## 5. 决策清单

| 需求 | C++17 写法 | C++20 写法 |
|---|---|---|
| 看对象的字节 | `unsigned char*` / `std::byte*` 视角 | 同左 |
| 位模式互换（float↔int 等） | `memcpy` 助手（示例模板） | `std::bit_cast` |
| 字节缓冲 → 结构体（反序列化） | `memcpy` 出值；校验 `sizeof` 与 `trivially copyable` | `std::bit_cast` |
| 指针 ↔ 整数 | `reinterpret_cast`（合法场景） | 同左 |
| 「把 X* 当 Y* 用」 | 不许；需要则回 `void*`/`memcpy`/重新设计 | 同左 |

## 本章要点

- `reinterpret_cast` 解引用跨类型读、union 读非活动成员，都是 UB。
- 合法双关只有两条路：`memcpy` 复制对象表示；字节类型视角查看。
- 双关双方必须 `trivially copyable` 且大小一致，`static_assert` 提前拦。
- C++20 `std::bit_cast` 取代 memcpy 助手，值级转换；指针伪转换别换。

## 待深入

- C++20 隐式生命周期对象（P0593）：`memcpy` 到裸内存何时「开始」对象的生命周期。
- `start_lifetime_as`（C++23 方向）与序列化器实现的关系。
