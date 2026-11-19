---
title: C++17 类型转换
series: cpp17-type-conversion
topic: 从隐式转换规则到命名 cast、RTTI 实现、类型双关与 UE 反射转换的完整深入
audience: 已会写 C++ 类和函数，但说不清「这里发生了什么转换」的开发者；游戏方向加分
outcome: 任意转换代码都能指出语境与风险等级；四个命名 cast 信手选对；热路径与 UE 场景知道替代方案
environment: C++17；CMake 3.16+；Windows + MSVC 19.50 已验证；2026-11-19
date: 2026-11-19
---
# C++17 类型转换

适用环境：C++17、CMake 3.16 或更高版本；示例已在 Windows + MSVC 19.50 下构建并通过全部测试。

你将学会：

- 指出转换发生的六个语境，并按四档风险阶梯给转换分级。
- 预判任意算术表达式的结果类型，避开 `-1 < 1u`、`size_t` 比较等符号坑。
- 用「标准 → 至多一个用户转换 → 标准」解释隐式链，用 `explicit` 设计类型接口。
- 按语义选对 `static_cast` / `dynamic_cast` / `const_cast` / `reinterpret_cast`，说清 C 风格 cast 的两条危害。
- 理解 `dynamic_cast` 的 RTTI 实现与多继承 `this` 指针调整，知道热路径的替代方案。
- 写不违反严格别名规则的位级转换（`memcpy`，C++20 换 `bit_cast`）。
- 解释 UE `Cast<T>` 用反射替代 `dynamic_cast` 的机制，正确选用 `Cast` / `CastChecked`。

## 章节目录

| 章节 | 主题 | 代码 |
|---|---|---|
| [01 转换全景](01-overview.md) | 两条路线、六个语境、风险阶梯 | [`01_overview.cpp`](code/01_overview.cpp) |
| [02 隐式转换规则](02-implicit-conversions.md) | 整型提升、算术转换、窄化、符号坑 | [`02_implicit.cpp`](code/02_implicit.cpp) |
| [03 用户自定义转换与 explicit](03-user-defined-conversions.md) | 转换构造/运算符、三段式序列 | [`03_user_defined.cpp`](code/03_user_defined.cpp) |
| [04 命名 cast 四剑客与 C 风格 cast](04-named-casts.md) | 四剑客语义边界、C 风格禁用原因 | [`04_named_casts.cpp`](code/04_named_casts.cpp) |
| [05 dynamic_cast 深入](05-dynamic-cast-internals.md) | 下行/跨类转换、this 调整、成本 | [`05_dynamic_cast.cpp`](code/05_dynamic_cast.cpp) |
| [06 类型双关与严格别名](06-type-punning-aliasing.md) | UB 双关、memcpy 正解、bit_cast 展望 | [`06_type_punning.cpp`](code/06_type_punning.cpp) |
| [07 UE Cast 专题](07-ue-casting.md) | 反射式转换、Cast/CastChecked 选型 | [`07_ue_cast_sim.cpp`](code/07_ue_cast_sim.cpp) |
| [08 最佳实践与综合练习](08-best-practices.md) | 决策清单、规范速查、五道综合练习 | [`08_exercise.cpp`](code/08_exercise.cpp) |

## 构建并运行

在仓库根目录执行：

```bash
cmake -S tutorials/cpp17-type-conversion/code -B .tmp/cpp17-cast-build
cmake --build .tmp/cpp17-cast-build --config Debug
ctest --test-dir .tmp/cpp17-cast-build -C Debug --output-on-failure
```

Visual Studio 等多配置生成器需要配置参数；Ninja 等单配置生成器可以省略 `--config Debug` 和 `-C Debug`。

每章代码均为可独立运行的演示（含断言验证），第 05 章附运行期成本对比，第 07 章是 UE Cast 语义的最小模拟（非 UE 源码）。

调研来源与验证记录见 [调研记录](resources/research.md)；结论已按笔记模板回写 `dx/cpp-learning/knowledge/type-conversion.md`。
