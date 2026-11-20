#include <cassert>
#include <iostream>
#include <iterator>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#define REQUIRE(cond)                                                          \
    do {                                                                       \
        if (!(cond)) {                                                         \
            std::cerr << "REQUIRE failed: " #cond " at line " << __LINE__      \
                      << '\n';                                                 \
            std::abort();                                                      \
        }                                                                      \
    } while (0)

// ---------------------------------------------------------------------------
// 1. 变参模板 + 折叠表达式：任意数量的参数
// ---------------------------------------------------------------------------

// 一元左折叠：(init + ... + pack)
template <typename... Args>
constexpr int sum(Args... values) {
    return (0 + ... + values);
}

// 一元右折叠到流：<< 结合性从右往左，避免括号歧义。
template <typename... Args>
void log_line(Args&&... parts) {
    std::cout << "[log] ";
    (std::cout << ... << std::forward<Args>(parts));
    std::cout << '\n';
}

// 对参数包逐项调用（包展开 + 逗号表达式丢弃返回值）。
template <typename... Systems>
std::size_t invoke_all(Systems&&... systems) {
    (systems.update(), ...); // 逗号折叠：按顺序逐个执行
    return sizeof...(systems);
}

struct FpsSystem {
    void update() { ++calls; }
    int calls = 0;
};
struct AiSystem {
    void update() { ++calls; }
    int calls = 0;
};

void variadic_basics() {
    static_assert(sum(1, 2, 3, 4) == 10, "折叠表达式在编译期也可用");
    REQUIRE(sum() == 0); // 空包：必须提供初值 0，否则 (pack + ...) 是病构
    REQUIRE(sum(5, -2) == 3);
    log_line("frame ", 16, " ms, entities ", 1024);

    FpsSystem fps;
    AiSystem ai;
    REQUIRE(invoke_all(fps, ai) == 2);
    REQUIRE(fps.calls == 1 && ai.calls == 1);
}

// ---------------------------------------------------------------------------
// 2. 手写 type traits：void_t 探测与 is_one_of（先声明，后面函数使用）
// ---------------------------------------------------------------------------

// 经典 void_t 技法：能对 T 调 .size() 的类型命中特化版本。
template <typename, typename = void>
struct has_size : std::false_type {};
template <typename T>
struct has_size<T, std::void_t<decltype(std::declval<T&>().size())>>
    : std::true_type {};

// 变参 traits：T 是否属于给定类型集合。
template <typename T, typename... Ts>
struct is_one_of : std::bool_constant<(std::is_same_v<T, Ts> || ...)> {};

static_assert(has_size<std::vector<int>>::value, "vector 有 size()");
static_assert(!has_size<int>::value, "int 没有 size()");
static_assert(is_one_of<float, int, float, double>::value, "float 在集合中");
static_assert(!is_one_of<char, int, float>::value, "char 不在集合中");

// void_t 的意义：抹掉返回类型差异，只问「能不能调」。反证：去掉 void_t，
// 特化只匹配「size() 返回类型恰好是 void」的类型，vector 反而不命中。
struct Weird { void size(); };
template <typename, typename = void>
struct only_void_size : std::false_type {};
template <typename T>
struct only_void_size<T, decltype(std::declval<T&>().size())> : std::true_type {};
static_assert(only_void_size<Weird>::value, "size() 返回 void，恰好对上");
static_assert(!only_void_size<std::vector<int>>::value, "vector 不加 void_t 反而不命中");

// void_t 可放多个探针，全部合法才算命中。
template <typename, typename = void>
struct sized_and_iterable : std::false_type {};
template <typename T>
struct sized_and_iterable<T, std::void_t<
    decltype(std::declval<T&>().size()),
    decltype(std::declval<T&>().begin())>> : std::true_type {};
static_assert(sized_and_iterable<std::vector<int>>::value, "size + begin 都有");
static_assert(!sized_and_iterable<Weird>::value, "有 size 没有 begin");

// ---------------------------------------------------------------------------
// 3. if constexpr：编译期分支，替代大部分经典 SFINAE
// ---------------------------------------------------------------------------

// 按类型类别走不同实现，全部写在同一个模板里。
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

// 有 size() 成员就返回 size，否则返回 0；未实例化的分支不做语义检查
// （只要它依赖模板参数 T）。
template <typename T>
auto size_or_zero(const T& container) {
    if constexpr (has_size<T>::value) {
        return container.size();
    } else {
        return 0;
    }
}

void if_constexpr_basics() {
    REQUIRE(describe(7) == "integral:7");
    REQUIRE(describe(1.5) == "floating:1.500000");
    REQUIRE(describe(std::string{"x"}) == "other");

    std::vector<int> v{1, 2, 3};
    REQUIRE(size_or_zero(v) == 3);
}

// ---------------------------------------------------------------------------
// 4. enable_if / SFINAE：约束模板的旧机制（读懂旧代码必备）
// ---------------------------------------------------------------------------

// 只有整数类型才参与重载决议；浮点参数时该模板被 SFINAE 移除。
template <typename T,
          typename = std::enable_if_t<std::is_integral_v<T>>>
int score_of(T value) {
    return static_cast<int>(value) * 2;
}

// SFINAE 旧写法：两个互补约束的孪生重载（if constexpr 出现前的标准形状）。
// 条件互补保证任何类型恰好命中一个；int = 0 非类型参数用于区分两个函数模板。
template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
std::string describe_sfinae(const T& value) { return "integral:" + std::to_string(value); }
template <typename T, std::enable_if_t<!std::is_integral_v<T>, int> = 0>
std::string describe_sfinae(const T&) { return "other"; }  // 不用参数就不命名

// tag dispatch：用类型标签选择实现，运行期零开销。
struct forward_tag {};
struct random_access_tag {};

constexpr forward_tag select_tag(std::forward_iterator_tag) {
    return {};
}
constexpr random_access_tag select_tag(std::random_access_iterator_tag) {
    return {};
}

template <typename Iter>
auto category_of(Iter) {
    using cat = typename std::iterator_traits<Iter>::iterator_category;
    return select_tag(cat{}); // 返回类型由重载决议选出的 select_tag 决定
}

void sfinae_and_dispatch() {
    REQUIRE(score_of(21) == 42);
    // score_of(1.5); 编译错误：enable_if 把浮点类型从候选里移除。

    // 孪生重载版：与 if constexpr 版行为一致
    REQUIRE(describe_sfinae(21) == "integral:21");
    REQUIRE(describe_sfinae(1.5) == "other");

    std::vector<int> v{1, 2, 3};
    static_assert(
        std::is_same<decltype(category_of(v.begin())), random_access_tag>::value,
        "vector 迭代器是随机访问");
}

int main() {
    variadic_basics();
    if_constexpr_basics();
    sfinae_and_dispatch();
    std::cout << "ch03_compile_time: all checks passed\n";
    return 0;
}
