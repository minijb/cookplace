---
title: C++17 RAII 与异常安全
series: cpp17-raii
topic: 从对象生命周期到资源包装器、异常安全和游戏资源管理
audience: 已掌握 C++ 基础，希望深入资源管理与异常路径的开发者
outcome: 能设计构造、析构、拷贝和移动语义正确的资源类型，并提供明确的异常安全保证
environment: C++17；CMake 3.16+；Windows + MSVC 19.50 已验证；2026-11-12
date: 2026-11-12
---
# C++17 RAII 与异常安全

适用环境：C++17、CMake 3.16 或更高版本；示例已在 Windows + MSVC 19.50 下构建并通过测试。

你将学会：

- 把资源可用性建立为类不变量，并精确理解对象销毁时机。
- 在构造失败和异常栈展开时释放已取得的资源。
- 编写不抛异常的析构函数，以及具有明确拷贝/移动语义的资源包装器。
- 区分基本、强和不抛异常保证。
- 用 RAII 设计游戏资源管理器，并识别引用与异步生命周期边界。

## 章节目录

| 章节 | 主题 | 代码 |
|---|---|---|
| [01 生命周期与类不变量](01-lifetime-and-invariants.md) | RAII 本质、资源类型、成员逆序销毁 | [`01_lifetime.cpp`](code/01_lifetime.cpp) |
| [02 构造失败与栈展开](02-construction-and-unwinding.md) | 部分构造、成员清理、工厂与失败路径 | [`02_construction.cpp`](code/02_construction.cpp) |
| [03 析构函数与异常](03-destructors-and-exceptions.md) | 析构时机、`noexcept`、显式关闭、多态销毁 | [`03_destructors.cpp`](code/03_destructors.cpp) |
| [04 拷贝、移动与所有权](04-copy-move-and-ownership.md) | Rule of Zero/Five、深拷贝、转移与移后状态 | [`04_copy_move.cpp`](code/04_copy_move.cpp) |
| [05 异常安全与组合](05-exception-safety-and-composition.md) | 基本/强/不抛保证、先准备再提交 | [`05_exception_safety.cpp`](code/05_exception_safety.cpp) |
| [06 游戏资源管理实战](06-game-resource-case.md) | GPU 句柄、独占资源、管理器和异步边界 | [`06_game_resources.cpp`](code/06_game_resources.cpp) |

## 构建并运行

在仓库根目录执行：

```bash
cmake -S tutorials/cpp17-raii/code -B .tmp/cpp17-raii-build
cmake --build .tmp/cpp17-raii-build --config Debug
ctest --test-dir .tmp/cpp17-raii-build -C Debug --output-on-failure
```

Visual Studio 等多配置生成器需要配置参数；Ninja 等单配置生成器可以省略 `--config Debug` 和 `-C Debug`。

调研来源与验证记录见 [调研记录](resources/research.md)。
