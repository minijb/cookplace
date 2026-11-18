---
title: 第三章（中）：变参模板与编译期分支
series: cpp17-game-dev
chapter: 第三章（中）
topic: 参数包、折叠表达式、if constexpr 与 type traits 的实用组合
audience: 已学习第三章（上），要读懂引擎模板代码的前端游戏开发工程师
outcome: 能用变参模板与折叠表达式写可变参数函数，用 if constexpr 与 traits 做编译期分派
environment: C++17；CMake 3.16+；Windows + MSVC 19.50 已验证；2026-11-18
date: 2026-11-18
---
# 第三章（中）：变参模板与编译期分支

适用环境：C++17、CMake 3.16 或更高版本；示例已在 Windows + MSVC 19.50 下验证。

你将学会：

- 用参数包与折叠表达式处理任意数量的参数。
- 用 `if constexpr` 把「按类型选择实现」写进同一个函数。
- 手写 `void_t` 探测与 `is_one_of` 两类常见 traits。
- 读懂旧代码里的 `enable_if`/SFINAE 与 tag dispatch。

完整代码位于 [编译期编程示例](../code/08-compile-time/main.cpp)。

## 1. 参数包与折叠表达式

变参模板用 `typename...` 声明参数包，C++17 的折叠表达式把「对包里每个元素做同一个运算」压缩成一行：

```cpp
template <typename... Args>
constexpr int sum(Args... values) {
    return (0 + ... + values);   // 一元左折叠：0 + v1 + v2 + ... + vn
}
```

四种折叠形态记法：`(pack op ...)`、`(... op pack)`、`(pack op ... op init)`、`(init op ... op pack)`。带 `init` 的版本可以在空包时也有定义——`sum()` 返回 `0`，而空包上的一元折叠只对 `&&`、`||`、`,` 有合法结果。

配合完美转发，可以写日志、事件触发这类「转发一切」的函数：

```cpp
template <typename... Args>
void log_line(Args&&... parts) {
    (std::cout << ... << std::forward<Args>(parts));  // 右折叠进流
}
```

对包逐项调用则用逗号折叠：

```cpp
template <typename... Systems>
std::size_t invoke_all(Systems&&... systems) {
    (systems.update(), ...);   // 按顺序逐个 update
    return sizeof...(systems); // sizeof... 求包长度
}
```

引擎里的典型用途：逐个调用子系统 `update`、把组件列表一次性转发给构造函数、变参事件派发。

## 2. `if constexpr`：编译期分支

`if constexpr` 在编译期丢弃不成立的分支，让「按类型选实现」写在同一个函数里：

```cpp
template <typename T>
std::string describe(const T& value) {
    if constexpr (std::is_integral_v<T>) {
        return "integral:" + std::to_string(value);
    } else if constexpr (std::is_floating_point_v<T>) {
        return "floating:" + std::to_string(value);
    } else {
        return "other";
    }
}
```

它替代的经典 SFINAE 场景：旧代码为了「有某成员走 A 路径，否则走 B 路径」要写两个互相约束的重载；C++17 只需一个函数加一个编译期条件。被丢弃的分支**不参与实例化语义检查**（只要依赖模板参数），因此可以有「这个类型走不到」的调用：

```cpp
template <typename T>
auto size_or_zero(const T& container) {
    if constexpr (has_size<T>::value) {
        return container.size();
    } else {
        return 0;
    }
}
```

> ⚠️ 「丢弃分支不检查」只对依赖模板参数的代码成立。分支里写一个与模板参数无关的语法错误，仍然会在定义处报错。

## 3. 手写 type traits

traits 是「关于类型的函数」，返回编译期常量或类型。两类最常用的手写 traits：

**探测成员**——`void_t` 技法：能对 `T` 调 `.size()` 的类型命中特化版本：

```cpp
template <typename, typename = void>
struct has_size : std::false_type {};

template <typename T>
struct has_size<T, std::void_t<decltype(std::declval<T&>().size())>>
    : std::true_type {};

static_assert(has_size<std::vector<int>>::value);
static_assert(!has_size<int>::value);
```

原理：`std::void_t<...>` 把任意合法表达式映射成 `void`；若 `decltype` 里的表达式非法，SFINAE 让这个特化从候选中移除，回退到主模板。

**类型集合判断**——变参 traits 用折叠表达式一行写完：

```cpp
template <typename T, typename... Ts>
struct is_one_of : std::bool_constant<(std::is_same_v<T, Ts> || ...)>;
```

引擎源码里大量出现这两类形态（UE 的 `TIsDerivedFrom`、检测 `GetTypeHash` 是否存在等），认出结构就能读懂。

## 4. `enable_if` 与 SFINAE：读懂旧代码

SFINAE（Substitution Failure Is Not An Error）指：替换模板参数导致的失败不是错误，只是把该候选从重载集合移除。`enable_if` 是它的标准封装：

```cpp
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
int score_of(T value) {
    return static_cast<int>(value) * 2;
}
// score_of(21)   OK
// score_of(1.5)  编译错误：浮点类型被移出候选，无可行函数
```

C++17 的实践建议：

- **新代码**：能用 `if constexpr` 就不用 SFINAE——一个函数、一眼读懂。
- **读旧代码**：看到 `enable_if` 就理解为「类型约束」，找出 `condition` 是什么即可，不必逐字推导替换过程。
- **约束接口**：需要让「不满足条件的重载」继续留在候选里让别的重载接住时（`if constexpr` 做不到跨函数分派），SFINAE 仍是正解。

## 5. tag dispatch：用类型选择实现路径

另一种编译期分派是给函数传一个**空类型标签**，让重载决议选择实现：

```cpp
struct forward_tag {};
struct random_access_tag {};

constexpr forward_tag       select_tag(std::forward_iterator_tag) { return {}; }
constexpr random_access_tag select_tag(std::random_access_iterator_tag) { return {}; }

template <typename Iter>
auto category_of(Iter) {
    using cat = typename std::iterator_traits<Iter>::iterator_category;
    return select_tag(cat{});   // 返回类型由重载决议决定
}
```

`vector` 的迭代器带 `random_access_iterator_tag`，于是 `category_of` 返回 `random_access_tag`。运行期零开销——标签只是编译期的「分支标记」。算法库内部（`std::advance` 对双向与随机访问迭代器走不同实现）就是这个模式，引擎的任务系统里也常见「串行 tag / 并行 tag」的重载。

## 6. 何时停手：元编程的度

模板是引擎源码的通用语，但教程目标定在「读懂」而不是「炫技」：

- 该用：容器与算法的泛型化、变参转发、`if constexpr` 类型分派、存在性探测。
- 慎用：深度递归元编程、为两行代码引入一整套 traits 体系、用模板实现本可用运行期多态或查表的逻辑。
- 代价指标：编译时间、报错长度、同事读懂所需时间。三者同时超标就停手。

## 7. 常见误区

- 在空参数包上写一元折叠：`(pack + ...)` 没有初值，空包直接编译错误；用 `(0 + ... + pack)`。
- 把「丢弃分支不做语义检查」扩大到与模板参数无关的错误。
- 读到 `enable_if` 就重写整个调用链：多数时候只需理解约束条件本身。
- 用元编程替代简单查表：编译期算十个常量没问题，编译期生成状态机就是另一个量级。

## 8. 练习

1. 给 `sum` 写空包调用，分别尝试一元与二元折叠，记录编译器报错差异。
2. 写 `has_reserve<T>` 探测 `reserve(size_t)` 成员，用 `static_assert` 验证 `vector` 为真、`list` 为假。
3. 用 `if constexpr` 写一个 `serialize(const T&)`：整数走 `to_string`，浮点走定点缩放，容器类型逐元素递归。
4. 把 `describe(std::string)` 的普通函数重载删掉，观察现在 `describe(std::string{"hi"})` 落到哪个候选，解释原因。
5. 在引擎源码里找一处 `enable_if` 或 `void_t` 的真实用法，用自己的话写出它约束了什么。

## 9. 小结

- 折叠表达式把对参数包的逐项运算压成一行；空包要带初值。
- `if constexpr` 是编译期分支，替代大部分 SFINAE 场景；丢弃分支需依赖模板参数。
- `void_t` 探测成员、变参折叠判断类型集合，是读引擎源码的两类高频结构。
- `enable_if` 是类型约束；新代码优先 `if constexpr`，旧代码读懂即可。
- tag dispatch 用空类型标签做零开销的编译期分派。

## 参考资料

- [cppreference：折叠表达式](https://en.cppreference.com/w/cpp/language/fold)
- [cppreference：`if constexpr`](https://en.cppreference.com/w/cpp/language/if)
- [cppreference：`std::enable_if`](https://en.cppreference.com/w/cpp/types/enable_if)
- [cppreference：`std::void_t` 探测技法说明](https://en.cppreference.com/w/cpp/types/void_t)
- [Template Metaprogramming with C++：Type Traits 与 SFINAE 章节](https://www.oreilly.com/library/view/template-metaprogramming-with/9781803243450/B18367_05_ePub.xhtml)
- 第三章调研与确认记录：[ch03-research](../resources/ch03-research.md)
