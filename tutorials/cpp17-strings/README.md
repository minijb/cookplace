---
title: C++17 字符串使用与优化
series: cpp17-strings
topic: 从字符串类型、文本语义和内存机制到游戏开发中的字符串优化
audience: 已掌握 C++ 基础，希望系统掌握字符串使用、UTF-8 和性能优化的开发者
outcome: 能正确选择字符串类型、处理常见文本操作和 UTF-8，并通过测量减少分配与拷贝
environment: C++17；CMake 3.16+；Windows + MSVC 19.50 已验证；2026-11-12
date: 2026-11-12
---
# C++17 字符串使用与优化

适用环境：C++17、CMake 3.16 或更高版本。示例已在 Windows + MSVC 19.50 下构建并通过测试，MSVC 使用 `/utf-8` 保证源码按 UTF-8 读取。

你将学会：

- 根据所有权选择 `std::string`、`std::string_view` 或 `const char*`。
- 正确使用查找、截取、拼接、数值转换和 UTF-8 文本。
- 理解 SSO、容量、移动和失效规则。
- 用 `pmr` 和基准测试处理游戏开发中的字符串分配。

## 章节目录

| 章节 | 主题 | 代码 |
|---|---|---|
| [01 字符串类型与接口设计](01-types-and-interfaces.md) | 所有权、参数和存储类型 | [`01_types.cpp`](code/01_types.cpp) |
| [02 常用操作](02-core-operations.md) | 访问、查找、截取、修改、比较 | [`02_operations.cpp`](code/02_operations.cpp) |
| [03 视图与生命周期](03-views-and-lifetimes.md) | 切分、去除空白、悬垂与失效 | [`03_views.cpp`](code/03_views.cpp) |
| [04 UTF-8 与文本处理](04-utf8-text.md) | 字节、码点、字素簇与编码边界 | [`04_utf8.cpp`](code/04_utf8.cpp) |
| [05 内存与移动](05-memory-and-move.md) | SSO、容量、`reserve`、`resize`、移动 | [`05_memory.cpp`](code/05_memory.cpp) |
| [06 拼接与数值转换](06-building-and-conversion.md) | `append`、`to_chars`、`from_chars` | [`06_conversion.cpp`](code/06_conversion.cpp) |
| [07 游戏开发优化](07-game-dev-optimization.md) | 帧内分配、`pmr::string`、内存资源 | [`07_pmr.cpp`](code/07_pmr.cpp) |
| [08 基准测试与综合练习](08-benchmarks-and-practice.md) | 性能测量、对照实验与综合项目 | [`08_benchmark.cpp`](code/08_benchmark.cpp) |

## 构建并运行

在仓库根目录执行：

```bash
cmake -S tutorials/cpp17-strings/code -B .tmp/cpp17-strings-build
cmake --build .tmp/cpp17-strings-build --config Debug
ctest --test-dir .tmp/cpp17-strings-build -C Debug --output-on-failure
```

使用 Visual Studio 等多配置生成器时保留 `--config Debug` 和 `-C Debug`；使用 Ninja 等单配置生成器时可以省略。性能测试请按第 08 章改用 Release 配置。

调研来源与验证状态见 [调研记录](resources/research.md)。
