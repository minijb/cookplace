---
title: 08 基准测试与综合练习
series: cpp17-strings
chapter: 08
topic: 通过可重复对照实验验证字符串优化，并完成游戏文本综合练习
audience: 已完成本专题前七章的 C++ 开发者
outcome: 能设计正确的字符串基准、解释测量限制，并综合应用类型选择和分配优化
environment: C++17；CMake 3.16+；Release 测量；Windows + MSVC 19.50 已验证；2026-11-12
date: 2026-11-12
---
# 08 基准测试与综合练习

适用环境：C++17；配套代码为 [`08_benchmark.cpp`](code/08_benchmark.cpp)。性能结论应在 Release 配置下验证。

## 1. 一个有效的对照实验

配套程序比较两种标签构造方式。未预留版本逐步增长：

```cpp
std::string build_without_reserve(std::string_view name, int score) {
    std::string result;
    result += "player:";
    result += name;
    result += " score:";
    result += std::to_string(score);
    return result;
}
```

预留版本先计算输出规模：

```cpp
std::string build_with_reserve(std::string_view name, int score) {
    const std::string score_text = std::to_string(score);
    std::string result;
    result.reserve(7 + name.size() + 7 + score_text.size());
    result.append("player:");
    result.append(name.data(), name.size());
    result.append(" score:");
    result.append(score_text);
    return result;
}
```

两条路径都调用 `std::to_string`，因此主要差异是结果字符串的增长策略。实际项目中还可以进一步使用第 06 章的 `to_chars` 消除数值临时字符串。

## 2. 防止测试变成无效比较

程序累计两个版本的输出长度，并在最后检查一致：

```cpp
if (checksum_without != checksum_with) {
    std::cerr << "benchmark paths produced different results\n";
    return 1;
}
```

一个基准至少需要：

1. 相同输入和相同语义输出。
2. 对结果进行可验证消费，避免循环变成无效工作。
3. 足够的迭代次数和多次独立运行。
4. 明确的编译器、标准库、优化级别和硬件环境。
5. 分别报告分配、耗时或内存指标，不用一个指标替代全部结论。

## 3. 使用 Release 构建

```bash
cmake -S tutorials/cpp17-strings/code -B .tmp/cpp17-strings-release -DCMAKE_BUILD_TYPE=Release
cmake --build .tmp/cpp17-strings-release --config Release
ctest --test-dir .tmp/cpp17-strings-release -C Release --output-on-failure
```

Visual Studio 等多配置生成器使用 `--config Release`；单配置生成器读取 `CMAKE_BUILD_TYPE`。不要根据 Debug 结果推断发布版本的字符串性能。

为保证 Release 示例仍执行正确性检查，本教程的 CMake 会移除默认 `NDEBUG` 标志，使 `assert` 保持有效；这是教学测试工程的选择，不是生产项目必须沿用的全局配置。

## 4. 视图与拥有子串的对比

```cpp
const std::string record = "player=alice;level=12";
const auto position = record.find('=');
const std::string owning_copy = record.substr(0, position);
const std::string_view non_owning_view(record.data(), position);
if (owning_copy != non_owning_view) {
    return 1;
}
```

两者内容相同，但成本和生命周期不同：

- `owning_copy` 拥有独立字符，可安全跨越源字符串生命周期。
- `non_owning_view` 不复制字符，但依赖 `record` 存活且不失效。

若后续必须长期保存，视图只是把复制推迟，并没有消灭复制需求。

## 5. 常见测量误区

- 用 SSO 能容纳的短字符串推导长字符串行为。
- 把一次运行中的毫秒差当成稳定结论。
- 忽略分配器预热、缓存状态和输入分布。
- 只测构造，不测真实业务中的查找、复制和销毁。
- 为消除复制引入悬垂视图。
- 只关注平均时间，不关注帧内尖峰和内存峰值。

性能数字只适用于所测环境；教程不提供固定加速比承诺。

## 6. 综合项目：战斗日志构建与解析

实现一个命令行小程序，输入多条记录：

```text
alice,1200;bob,800;carol,1500
```

要求：

1. 使用 `string_view` 按分号和逗号切分，不复制临时字段。
2. 使用 `from_chars` 解析分数，拒绝空名称、无效数字和尾随字符。
3. 用 `pmr::string` 构造本批格式化输出，并统一释放内存资源。
4. 对输出规模合理 `reserve`，数值转换使用 `to_chars`。
5. 若要跨批保存玩家名称，将其转换为拥有 `string`。
6. 为 UTF-8 玩家名称明确长度限制采用字节、码点还是字素簇。
7. 编写正确性测试，再比较默认字符串与优化版本的分配和耗时。

> ⚠️ 输入文本必须比本批视图活得更久；内存资源必须比所有 `pmr::string` 活得更久。

## 7. 最终检查清单

- 类型是否准确表达所有权？
- 查找是否处理 `npos`，访问是否越界？
- 视图是否可能悬垂或被错误当成 C 字符串？
- 是否区分字节、码点和字素簇？
- 是否区分 `reserve`、`resize` 和非强制的 `shrink_to_fit`？
- 是否检查了数值转换错误和完整消费？
- 是否通过 Release 测量确认优化价值？
- 优化后是否仍满足生命周期与输出正确性？

## 小结

字符串优化不是单个 API 技巧，而是“所有权 → 文本语义 → 分配策略 → 测量”的完整过程。正确性和生命周期始终优先于局部性能数字。

## 参考资料

- [cppreference：`std::basic_string`](https://en.cppreference.com/cpp/string/basic_string)
- [cppreference：`std::string_view`](https://en.cppreference.com/cpp/string/basic_string_view)
- [cppreference：`to_chars`](https://en.cppreference.com/cpp/utility/to_chars)
- [cppreference：`from_chars`](https://en.cppreference.com/cpp/utility/from_chars)
- [字符串类型性能研究](https://douglasorr.github.io/2024-10-strings/article.html)
