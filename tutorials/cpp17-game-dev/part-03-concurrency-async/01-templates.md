---
title: 第三章（上）：模板基础与实参推导
series: cpp17-game-dev
chapter: 第三章（上）
topic: 从实参推导到 CTAD，读懂引擎源码的模板基本功
audience: 已学习第一、二章，准备阅读引擎源码的前端游戏开发工程师
outcome: 能推导模板实参、正确使用 CTAD 与非类型模板参数，并能定位模板重载失败的原因
environment: C++17；CMake 3.16+；Windows + MSVC 19.50 已验证；2026-11-18
date: 2026-11-18
---
# 第三章（上）：模板基础与实参推导

适用环境：C++17、CMake 3.16 或更高版本；示例已在 Windows + MSVC 19.50 下验证。

你将学会：

- 推导函数模板的实参，并说清按值与按引用传参的推导差异。
- 使用类模板实参推导（CTAD）与推导指引，避开 `initializer_list` 优先匹配的坑。
- 用非类型模板参数把编译期常量编进类型。
- 理解模板重载决议的顺序，能定位「匹配失败」的报错原因。
- 按头文件约定组织模板代码。

完整代码位于 [模板示例](../code/07-templates/main.cpp)。

## 1. 为什么游戏引擎离不开模板

引擎源码里模板无处不在：UE 的容器叫 `TArray`、`TMap`，前缀 `T` 就是 template。原因不是炫技，而是两个硬需求：

1. **零抽象开销**：`std::sort` 对 `int` 排序生成的代码与手写循环一样快——模板在编译期实例化，没有虚调用、没有函数指针。
2. **一份代码适配所有类型**：容器、智能指针、数学库（`TVector<float>` 与 `TVector<double>`）都不可能为每种类型手写一份。

第三章沿 roadmap 覆盖三块：模板（本课与下一课）、并发与原子（中下、下）、异步与线程池（实战）。模板部分的目标以「读懂引擎源码」为准：常见写法要能认出，复杂元编程能看懂意图即可。

## 2. 函数模板与实参推导

函数模板把类型参数化，实参类型由编译器推导：

```cpp
template <typename T>
T sum_by_value(T a, T b) {
    return a + b;
}
```

推导规则里最重要的一条是**按值传递会 decay**：顶屋 `const` 丢失，数组退化为指针，函数退化为函数指针。按 `const T&` 传递则保留类型信息：

```cpp
const int big = 10;
const int small = 3;

sum_by_value(big, small);  // T 推导为 int：顶屋 const 被 decay
max_by_ref(big, small);    // T 推导为 int，const 保留在参数类型里
```

> ⚠️ decay 只发生在按值传递的**参数类型**层面。`const int&` 传给 `T`（按值 `T a`）时 `T` 是 `int`，这不是「丢掉 const 语义」，而是「复制了一份新的、可修改的副本」。原对象的 const 从未改变。

另一个高频考点：`auto` 的推导规则与模板推导一致。`auto x = expr` 对应按值传参（decay），`auto& x = expr` 对应按引用传参。第一章里「`auto` 拷贝了容器」的坑，根源就是这条规则。

## 3. 类模板与 CTAD

C++17 之前，类模板必须显式写出模板实参；C++17 引入类模板实参推导（CTAD），让编译器从构造实参推导：

```cpp
ResourceCache names{std::string{"hero"}, std::string{"slime"},
                    std::string{"hero"}};
// 推导为 ResourceCache<std::string>，无需写 ResourceCache<std::string> names{...};
```

CTAD 优先匹配 `initializer_list` 构造函数，这是新手最容易踩的坑：

```cpp
std::vector<int> ids{1, 2, 3};
ResourceCache cached{ids};  // 错误预期：ResourceCache<int>
// 实际：ResourceCache<std::vector<int>>，整个 ids 成为唯一元素！
```

花括号初始化强烈偏好 `initializer_list` 构造函数，于是 `ids` 被当成「一个 `vector<int>` 元素」。要从可迭代范围推导，用括号初始化走普通构造函数：

```cpp
ResourceCache cached(ids);  // ResourceCache<int>，三个元素
```

> ⚠️ CTAD 的推导依据是**构造函数**。类没有匹配的构造函数时，可以补充推导指引（deduction guide），例如 `template <typename It> ResourceCache(It, It) -> ResourceCache<typename iterator_traits<It>::value_type>;`。标准库容器的推导指引已经内置。

## 4. 非类型模板参数：把常量编进类型

模板参数不一定是类型，也可以是编译期常量：

```cpp
template <typename T, std::size_t N>
class RingBuffer { /* 内部数组 T slots_[N] */ };

RingBuffer<int, 4> small_queue;
RingBuffer<int, 64> big_queue;  // 与前者是不同类型
```

`RingBuffer<int, 4>` 与 `RingBuffer<int, 64>` 是两个完全独立的类型，容量进入类型系统后：

- 编译器知道容量，可以做越界静态检查、完全内联。
- 不能再把「容量 4 的队列」赋给「容量 64 的队列」，错误在编译期暴露。

游戏里的典型用途：`std::array<T, N>`、固定容量的帧内事件队列、编译期表驱动配置。运行期才确定的容量（比如玩家可调设置）不该用非类型参数——那是构造函数参数的职责。

## 5. 重载、特化与「匹配失败」的定位

同名模板可以互相重载，决议顺序是：

1. 完全匹配的**普通函数**优先；
2. 其次在模板之间选「更特化」的那个；
3. 都不行才考虑用户定义转换、报二义性或匹配失败。

```cpp
template <typename T> const char* describe(const T&) { return "generic"; }
const char* describe(const std::string&) { return "string"; }          // 普通函数
template <typename T> const char* describe(const std::vector<T>&) {    // 更特化的模板
    return "vector";
}

describe(42);                              // "generic"
describe(std::string{"hi"});               // "string"：普通函数胜出
describe(std::vector<int>{1});             // "vector"：更特化的模板胜出
```

模板报错难读的根源是**两阶段处理**：编译器先在定义处检查「不依赖模板参数」的部分，再在每个实例化点检查其余部分。报错链条从「实例化这里」一路展开到「定义内部的深层失败」。定位方法：

- 从**第一条错误**读起，忽略后续雪崩；
- 找到最后一行的 `instantiation context`（实例化上下文），那里是触发点；
- 把 `enable_if`/SFINAE 类错误理解为「候选被移除」，不是「表达式非法」。

> ⚠️ `score_of(1.5)` 这类调用不是运行时错误，而是编译错误：`enable_if` 把浮点类型从重载候选中移除后，没有任何可行函数。SFINAE 的细节在下一课展开。

## 6. 模板代码放头文件，还是源文件

模板定义必须对每个实例化点可见，因此**模板通常整体放头文件**。变参模板、`constexpr` 函数同样如此。这带来两个实际后果：

1. 改任何一行模板实现，所有包含它的翻译单元都要重编——引擎项目里「改一行 TArray 全量重编」的痛感来源于此。
2. 头文件膨胀会拖慢编译。工程上的缓解手段：把不依赖模板参数的逻辑拆到 `.cpp`（比如模板类的非模板辅助函数）、减少不必要的模板实参组合、显式实例化声明（`extern template`）。

本教程的示例为了可读性把定义与使用放在同一个 `.cpp`，工程代码请遵守头文件约定。

## 7. 常见误区

- 认为按值传递「保留 const」：值传递复制对象，参数副本的 const 与原对象无关。
- 用 `{}` 初始化类模板并期望走普通构造函数：`initializer_list` 构造函数优先匹配。
- 把非类型模板参数当运行期配置：容量进类型就无法运行期改变。
- 看到模板报错就从头读到尾：雪崩式报错只有第一条和实例化上下文有诊断价值。

## 8. 练习

1. 写一个 `template <typename T> void decay_probe(T)`，用 `static_assert` 验证传 `const char[8]` 时 `T` 推导为 `const char*`。
2. 给 `ResourceCache` 补充一对迭代器范围的推导指引，验证 `ResourceCache r(ids.begin(), ids.end())` 能正确推导。
3. 实现 `RingBuffer<std::string, 8>`，观察两次实例化生成的代码差异（可用 `-fno-implicit-templates` 或链接错误感知独立实例）。
4. 故意写错一个模板实参（例如给 `RingBuffer<int, -1>`），阅读并摘录第一条错误与实例化上下文。
5. 找一份你熟悉的引擎容器头文件（如 UE `TArray.h`），数出其中出现的模板技法并标注属于本课哪一节。

## 9. 小结

- 模板是编译期代码生成：零抽象开销，代价是报错难读与编译时间。
- 按值推导会 decay，按引用推导保留类型；`auto` 与模板推导同规则。
- CTAD 从构造函数推导类模板实参；`{}` 会优先匹配 `initializer_list`。
- 非类型模板参数把编译期常量编进类型，容量不同即类型不同。
- 重载决议：普通函数 > 更特化的模板；报错从第一条与实例化上下文读起。

## 参考资料

- [cppreference：模板实参推导](https://en.cppreference.com/w/cpp/language/template_argument_deduction)
- [cppreference：CTAD 与推导指引](https://en.cppreference.com/w/cpp/language/class_template_argument_deduction)
- [LearnCpp：类模板与 CTAD](https://www.learncpp.com/cpp-tutorial/template-classes/)
- [C++ Templates: The Complete Guide 2e（目录）](https://www.oreilly.com/library/view/c-templates-the/9780134778808/ch17.xhtml)
- [C++17 - The Complete Guide（目录）](https://www.cppstd17.com/cppstd17.html)
- 第三章调研与确认记录：[ch03-research](../resources/ch03-research.md)
