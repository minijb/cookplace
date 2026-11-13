---
title: 03 视图与生命周期
series: cpp17-strings
chapter: 03
topic: 使用 string_view 进行零拷贝文本处理，同时保证底层存储有效
audience: 已掌握字符串基础和 string_view 接口选择的开发者
outcome: 能实现视图切分和裁剪，识别悬垂、失效及空字符结尾问题
environment: C++17；CMake 3.16+；Windows + MSVC 19.50 已验证；2026-11-12
date: 2026-11-12
---
# 03 视图与生命周期

适用环境：C++17；配套代码为 [`03_views.cpp`](code/03_views.cpp)。

## 1. 视图切片不拥有字符

`string_view::substr()` 只调整起点和长度，不复制字符。这适合解析协议、配置和战斗日志等临时文本。

裁剪 ASCII 空白可以写成：

```cpp
std::string_view trim_ascii(std::string_view text) {
    while (!text.empty() &&
           std::isspace(static_cast<unsigned char>(text.front())) != 0) {
        text.remove_prefix(1);
    }
    while (!text.empty() &&
           std::isspace(static_cast<unsigned char>(text.back())) != 0) {
        text.remove_suffix(1);
    }
    return text;
}
```

函数只修改自己的视图副本，不修改输入字符。`std::isspace` 接收前将 `char` 转为 `unsigned char`，避免负值带来的参数问题。这里只处理 ASCII/C 区域设置意义上的空白，不是 Unicode 空白算法。

## 2. 零拷贝切分

```cpp
std::vector<std::string_view> split_view(std::string_view text, char separator) {
    std::vector<std::string_view> parts;
    std::size_t begin = 0;
    while (begin <= text.size()) {
        const auto end = text.find(separator, begin);
        if (end == std::string_view::npos) {
            parts.push_back(text.substr(begin));
            break;
        }
        parts.push_back(text.substr(begin, end - begin));
        begin = end + 1;
    }
    return parts;
}
```

这个函数仍会为 `vector` 的元素数组分配内存，只是不复制每个子串的字符。因此“零拷贝”不等于“零分配”。

```cpp
const std::string input = "  alice,level-12,region-asia  ";
const std::string_view cleaned = trim_ascii(input);
const auto parts = split_view(cleaned, ',');

assert(parts.size() == 3);
assert(parts[0] == "alice");
assert(parts[1] == "level-12");
assert(parts[2] == "region-asia");
```

`input` 在所有视图使用期间保持存活且不变，因此结果有效。

## 3. 生命周期规则

视图安全使用需要同时满足：

1. 底层字符对象仍然存活。
2. 没有操作使对应字符存储失效。
3. 视图范围仍符合业务语义。

不要返回指向函数局部 `string` 的视图，也不要把视图保存到比源对象活得更久的成员中。需要跨生命周期保存时显式复制：

```cpp
const std::string saved_for_later(parts[1]);
assert(saved_for_later == "level-12");
```

视图特别适合函数参数和短生命周期局部变量。作为成员、返回值或容器元素时，应先证明生命周期关系。

## 4. 空字符结尾是另一项要求

```cpp
const std::string storage = "xxplayerYY";
const std::string_view view(storage.data() + 2, 6);
assert(view == "player");
assert(view.size() == 6);
```

`view` 的内容是 `player`，但视图结尾后面紧跟原字符串中的 `Y`，不是为视图单独准备的空字符。不能将 `view.data()` 直接交给要求读取完整 C 字符串的接口。

需要空字符结尾时，构造 `std::string(view)` 再使用其 `c_str()`；同时保证该拥有字符串在 C 调用期间不被修改。

## 5. 失效与悬垂不是同一个概念

- **悬垂**：底层对象已经销毁。
- **失效**：对象可能仍在，但扩容等操作使旧指针或视图不再指向有效预期范围。

`reserve()`、扩容式 `append()` 等操作可能重新分配字符存储。修改后需要重新取得视图；第 05 章会验证容量与指针变化。

## 6. 练习

1. 让 `split_view()` 保留或丢弃空字段，并明确连续分隔符的行为。
2. 将解析结果保存到跨帧存在的对象中，为每个字段选择 `string` 或 `string_view`。
3. 给 `trim_ascii()` 添加全空白输入测试。
4. 找出项目中把 `view.data()` 当作完整 C 字符串的接口，并修正。

## 小结

`string_view` 减少字符复制，但把生命周期责任交给调用方。先保证正确性，再利用视图降低开销。

## 参考资料

- [cppreference：`std::string_view`](https://en.cppreference.com/cpp/string/basic_string_view)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [C++ Stories：`string_view` 性能与边界](https://www.cppstories.com/2018/07/string-view-perf/)
