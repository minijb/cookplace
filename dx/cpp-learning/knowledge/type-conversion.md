---
title: 类型转换
brief: C++17 类型转换全貌：隐式转换规则、用户自定义转换与 explicit、命名 cast 四剑客、dynamic_cast 实现、类型双关与 UE 反射 Cast
tags: [cpp, core, type-system]
---
# 类型转换

基线 C++17。教程全文与代码验证见 `tutorials/cpp17-type-conversion/`（2026-11-19，MSVC 19.50 /W4 零警告，CTest 8/8）。

## 隐式转换规则
- 整型提升：小于 int 的类型（bool/char/short 等）运算前先提升为 int，值保持；`unsigned char + unsigned char` 的结果类型是 int，不回绕（实验验证）。
- usual arithmetic conversions：混合符号运算同秩时全体转无符号——`-1 < 1u` 为 `false`（-1 变 4294967295）；`0u - 1 == UINT_MAX`（实验验证）。出处：cppreference usual_arithmetic_conversions。
- 窄化转换：拷贝初始化放行（MSVC /W4 报 C4244），花括号列表初始化直接编译错误；实践：可能窄化处一律 `static_cast` 显式标注或花括号拦截。
- 浮点↔整型：浮点转整型向零截断，截断后超范围是 UB；整型转浮点可能丢精度（16777217 转 float 得 16777216，实验验证）。
- `size_t` 与 int 比较：int 先转无符号再比较，负值变巨大正数；比较前统一符号性。
- `void*` 往返：任意对象指针隐式转 void*，取回必须 `static_cast<T*>`，取错类型违反别名。

## 用户自定义转换与 explicit
- 隐式转换链三段式：标准转换 → 至多一个用户自定义转换（转换构造函数或转换运算符）→ 标准转换；两跳用户转换（A→B→C）病构。出处：cppreference implicit_conversion。
- 单参数构造函数默认写 `explicit`；仅「宽化且无歧义」的转换才值得隐式（string_view 从字面量构造是好的隐式）。MISRA C++:2023 R15.1.3、AUTOSAR A12-1-2 强制。
- `explicit operator bool` 是唯一推荐的隐式友好转换运算符：if/!/&&/||/?: 的语境转换豁免 explicit，算术语境封死。AUTOSAR A13-5-2 要求其余转换运算符一律 explicit。

## 命名 cast 四剑客
- 选型按语义：编译期语义明确 → `static_cast`（枚举↔整数、浮点↔整数、上行、void* 往返、调用 explicit 构造）；运行期验型 → `dynamic_cast`；去 const → `const_cast`（仅适配只读旧接口）；位重解释 → `reinterpret_cast`（仅指针↔整数与字节视角）。
- `static_cast` 下行转换无检查，类型错了是 UB；「类型已被证明」才允许，否则 dynamic_cast 或改设计。
- `const_cast` 去掉 const 后修改真正定义为 const 的对象是 UB；合法用途仅适配只读旧 C 接口与 const/非 const 重载去重。
- C 风格 cast 禁用：编译器按 const_cast → static_cast(带扩展，无视 private 继承) → +const_cast → reinterpret_cast(带扩展) → +const_cast 顺序静默尝试，代码上无从审计；实验验证 `(PrivBase*)&privDerived` 能绕过 private 继承而 static_cast 编译报错。出处：cppreference explicit_cast。

## dynamic_cast 实现
- 三种真动态（Itanium ABI）：下行转换、跨类转换（兄弟基类间）、`dynamic_cast<void*>`（取最派生地址，可用于同一对象判等）；上行转换编译期完成零成本。出处：cppreference、Itanium 讲义（见教程 resources/research.md）。
- 要求源类型多态（至少一个虚函数）；typeinfo 经虚表可达，`-fno-rtti`/`/GR-` 下 dynamic_cast/typeid 整体不可用。
- 多继承下同一对象有多个基类子对象地址（实测 Entity*/Weapon* 相差 16 字节）；跨类偏移只有运行期知道。
- 失败语义：指针返 nullptr，引用抛 std::bad_cast。
- 成本随继承图深度与兄弟数增长，非常数；实现差异大（libc++ 曾比 libstdc++ 慢 8–30 倍，后续修复）。本机 Debug 1×10⁷ 次 dynamic_cast 289 ms vs static_cast 7.4 ms（量级参考）。热路径替代：虚函数 > visitor/double dispatch > 已证明类型的 static_cast。

## 类型双关与严格别名
- `*reinterpret_cast<int*>(&float)` 违反严格别名规则是 UB；union 读非活动成员在 C++ 是 UB。编译器优化（-O2 别名分析）后会真实重排/删除访问。
- 合法双关仅两条路：`memcpy` 复制对象表示（双方须 trivially copyable 且大小一致，static_assert 前置拦截）；char/unsigned char/std::byte 指针视角查看任意对象（明文豁免）。
- C++20 `std::bit_cast`（constexpr）取代 memcpy 助手做值级转换；不得机械替换「把 X* 当 Y* 解引用」的指针伪转换（clang-tidy bugprone-bitwise-pointer-cast）。
- 与 `dx:cpp-learning/knowledge/object-layout.md` 呼应：trivially copyable + standard-layout 才可安全 memcpy/序列化。

## UE Cast（反射式转换）
- UE 以 /GR- 关 RTTI，dynamic_cast 编译不过；判型由 UClass 反射承担：`Cast<T>(Obj)` = IsChildOf 反射判定 + reinterpret_cast 落实（UObject 单继承链保证子对象偏移 0），失败返 nullptr。
- 成本：非编辑器构建近常数，编辑器 O(继承深度)。出处：UE 官方论坛验证帖、peterleontev 源码解析（见教程 resources/research.md）。
- 选型：类型可能不是 T → `Cast` 分支处理；契约必是 T → `CastChecked`（断言/崩溃）。UObject 层级禁用 static_cast 下行。
- 非 UObject 的原生 C++ 层无反射，回到标准工具（static_cast/类型标签/visitor）。

```cpp
// C++17：三行记住三个高危点
bool bad1 = (-1 < 1u);                          // false：-1 被转成无符号
// float f = 1.0f; int b = *reinterpret_cast<int*>(&f);  // UB：严格别名
// struct P { explicit P(int); }; P p = 42;     // 编译错误：explicit 封死隐式
```

## 坑
- `-1 < 1u` 为 false → 同秩混合比较前统一符号性；`vector::size()` 与 int 比较同理。
- `unsigned` 计数器 `i >= 0` 恒真回绕死循环 → 倒计数用有符号类型。
- C 风格 cast 静默绕过 private 继承/升级 reinterpret_cast → 全项目禁用，检索 `_cast` 可审计。
- 双关 UB「能跑」只是没开优化 → -O2 后别名分析会重排；一律 memcpy/bit_cast。
- 逐帧百万级判型用 dynamic_cast 是热路径事故 → 虚函数或 visitor。

## 待深入
- Itanium ABI 的 `__si_class_type_info`/`__vmi_class_type_info` 结构与虚继承下的 dynamic_cast 路径（回链 roadmap 贯穿专题·内存底层）
- C++20 `bit_cast`/隐式生命周期（P0593）与 C++23 `start_lifetime_as` 对序列化的影响（回链 roadmap 阶段 1 现代语法）
- UE UHT 生成的 `.generated.h` 与 `TScriptInterface` 双指针布局（回链 roadmap 阶段 5 UE 专题，见 `dx:cpp-learning/knowledge/unreal-engine.md`）
