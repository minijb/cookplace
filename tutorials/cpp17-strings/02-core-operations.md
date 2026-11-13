---
title: 02 常用操作
series: cpp17-strings
chapter: 02
topic: 正确执行字符串访问、查找、截取、修改和比较
audience: 已理解字符串所有权的 C++ 学习者
outcome: 能处理 npos、越界、子串范围和内嵌空字符，避免常见 API 误用
environment: C++17；CMake 3.16+；Windows + MSVC 19.50 已验证；2026-11-12
date: 2026-11-12
---
# 02 常用操作

适用环境：C++17；配套代码为 [`02_operations.cpp`](code/02_operations.cpp)。

## 1. 访问：长度、下标与检查

`size()` 和 `length()` 返回字符单元数量；对 `std::string` 而言就是 `char` 字节数。`empty()` 直接表达是否为空。

- `operator[]` 不执行边界检查，越界使用会导致未定义行为。
- `at()` 在位置越界时抛出 `std::out_of_range`。

```cpp
bool caught = false;
try {
    static_cast<void>(message.at(message.size()));
} catch (const std::out_of_range&) {
    caught = true;
}
assert(caught);
```

有效下标范围是 `0 <= index < size()`；`size()` 本身不是有效字符下标。

## 2. 查找：必须处理 `npos`

查找失败返回 `std::string::npos`，不是 `-1`，也不是 `0`：

```cpp
const std::string record = "player=alice;level=12";
const auto separator = record.find('=');
assert(separator != std::string::npos);
assert(record.find("missing") == std::string::npos);
```

位置类型应使用 `std::string::size_type`、`std::size_t` 或能保留该类型的 `auto`。在计算下一次查找起点前，先确认前一次查找成功。

## 3. 截取：理解位置和长度

`substr(pos, count)` 的第二项是最多截取的字符数，不是结束下标：

```cpp
const std::string key = record.substr(0, separator);
const auto end = record.find(';', separator + 1);
const std::string value = record.substr(separator + 1, end - separator - 1);
assert(key == "player");
assert(value == "alice");
```

`std::string::substr()` 返回新的拥有字符串。只需要临时读取切片时，可以改用 `string_view::substr()`，下一章会详细说明。

> ⚠️ `pos > size()` 会抛出 `std::out_of_range`；若 `end` 是 `npos`，不能直接参与普通范围计算。本例输入格式固定，真实解析器应先验证分隔符存在。

## 4. 修改：插入、删除与替换

```cpp
std::string message = "score:12";
message.insert(message.size(), ";combo:2");
assert(message == "score:12;combo:2");
message.erase(message.find(";combo"));
assert(message == "score:12");
message.replace(message.find("12"), 2, "20");
assert(message == "score:20");
```

这些操作可能改变长度并触发重新分配。不要跨越修改操作继续依赖之前保存的指针、引用或迭代器，除非已经核实对应失效规则。

频繁在字符串中间插入或删除通常意味着算法或数据结构需要重新评估；优化前先确认它是否真是热点。

## 5. 比较与二进制安全内容

```cpp
assert(record.compare(0, 6, "player") == 0);
```

`compare()` 返回负数、零或正数，判断相等时与 `0` 比较。直接比较字符串或视图通常更清晰。

字符串可以包含内嵌空字符，查找也会按显式长度处理：

```cpp
const std::string bytes("a\0b", 3);
assert(bytes.size() == 3);
assert(bytes.find(std::string_view("b", 1)) == 2);
```

这与 C 风格“遇空字符结束”的查找函数不同。

## 6. 练习

1. 编写解析器，将 `player=alice;level=12` 拆成两个键值对，并处理缺少分隔符的输入。
2. 分别用 `at()` 和 `operator[]` 访问边界，说明为什么发布版本不能依赖越界后的“正常表现”。
3. 将 `substr()` 的第二个参数误写为结束下标，观察结果并修正。
4. 构造包含两个空字符的字符串，验证 `size()`、`find()` 和 C 字符串 API 的差异。

## 小结

正确字符串处理依赖三个习惯：检查 `npos`、区分位置与长度、在修改后重新评估指针和视图有效性。

## 参考资料

- [cppreference：`std::basic_string`](https://en.cppreference.com/cpp/string/basic_string)
- [cppreference：`std::string_view`](https://en.cppreference.com/cpp/string/basic_string_view)
