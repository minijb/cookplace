---
title: 06 拼接与数值转换
series: cpp17-strings
chapter: 06
topic: 高效构造字符串，并使用 C++17 字符转换 API 处理数值
audience: 需要生成日志、UI 文本和解析配置的 C++ 开发者
outcome: 能减少拼接临时对象，正确使用 to_chars/from_chars 并检查转换结果
environment: C++17；CMake 3.16+；Windows + MSVC 19.50 已验证；2026-11-12
date: 2026-11-12
---
# 06 拼接与数值转换

适用环境：C++17；配套代码为 [`06_conversion.cpp`](code/06_conversion.cpp)。

## 1. 拼接的主要成本

字符串拼接可能产生三类成本：

- 为目标字符串重新分配。
- 创建中间字符串。
- 把已有字符重复复制到中间结果。

`operator+` 并不是天然低效；简单表达式清晰且实现可以利用右值操作。需要优化的是已确认的热点，尤其是循环和复杂多段构造。

## 2. 使用 `reserve` 与 `append`

```cpp
std::string build_player_label(std::string_view name, int score) {
    std::array<char, 32> number{};
    const auto conversion = std::to_chars(
        number.data(), number.data() + number.size(), score);
    assert(conversion.ec == std::errc{});

    std::string result;
    result.reserve(name.size() + 14 +
                   static_cast<std::size_t>(conversion.ptr - number.data()));
    result.append("player:");
    result.append(name.data(), name.size());
    result.append(" score:");
    result.append(number.data(), conversion.ptr);
    return result;
}
```

这里先完成数值转换，再根据各段长度一次性预留；`14` 是 `"player:"` 的 7 字节与 `" score:"` 的 7 字节之和。`append(const char* first, const char* last)` 使用指针范围，不要求数字缓冲区以空字符结尾。

示例用 `assert` 简化错误处理；生产接口应返回错误或采用明确的异常策略，不能在发布配置中依赖断言处理输入失败。

## 3. `to_chars`：向已有缓冲区写数值

C++17 的 `std::to_chars`：

- 不动态分配结果字符串。
- 不依赖区域设置。
- 通过 `to_chars_result` 返回结束指针和错误码，而不是抛出转换异常。

必须检查 `ec`，并确认缓冲区足够大。转换成功后，`ptr` 指向已写字符之后的位置，因此可用 `[buffer.data(), ptr)` 构造视图或追加内容。

`std::format` 不属于 C++17；若项目严格使用 C++17，应选现有格式化库或明确封装的转换函数。

## 4. `from_chars`：解析并验证完整输入

```cpp
bool parse_score(std::string_view text, int& score) {
    const auto result = std::from_chars(text.data(), text.data() + text.size(), score);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size();
}
```

需要同时检查：

1. `ec` 表示没有格式错误或越界错误。
2. `ptr` 到达输入末尾，避免把 `"1200 points"` 当成成功的纯数字。

```cpp
int score = 0;
assert(parse_score("1200", score));
assert(score == 1200);
assert(!parse_score("1200 points", score));
assert(!parse_score("not-a-number", score));
```

`from_chars` 不会替调用方实现业务规则，例如是否允许前导空白、正负号或特定进制；这些需要按接口文档和参数明确处理。

## 5. 字符串流何时仍合适

```cpp
std::ostringstream stream;
stream << "player:" << "alice" << " score:" << 1200;
assert(stream.str() == label);
```

字符串流提供丰富的格式化和区域设置能力，但对象和内部缓冲通常比简单数值转换更重。可维护性优先时可以使用；热路径则应通过测量决定。

`std::to_string` 适合简单、非热点转换；`to_chars` 更适合控制缓冲区和分配的场景。

## 6. 优化顺序

1. 先保证转换错误被处理。
2. 用直接、可读的接口完成初版。
3. 测量分配和临时对象。
4. 对热点使用 `reserve`、`append` 或 `to_chars`。
5. 保留对照测试，确保优化没有改变输出。

## 7. 练习

1. 让 `build_player_label()` 支持负数，并确认缓冲区估计仍安全。
2. 将断言改为返回 `std::optional<std::string>`。
3. 为 `parse_score()` 添加先裁剪 ASCII 空白的版本，并单独测试尾随字符。
4. 对比 `operator+`、字符串流和 `append` 的可读性与性能，不只比较单次运行时间。

## 小结

高效构造字符串的关键是控制分配和临时对象。数值转换还要完整检查错误码与消费范围，不能只验证“前缀能解析”。

## 参考资料

- [cppreference：`append`](https://en.cppreference.com/cpp/string/basic_string/append)
- [cppreference：`to_chars`](https://en.cppreference.com/cpp/utility/to_chars)
- [cppreference：`from_chars`](https://en.cppreference.com/cpp/utility/from_chars)
- [cppreference：文本处理库](https://en.cppreference.com/cpp/text)
