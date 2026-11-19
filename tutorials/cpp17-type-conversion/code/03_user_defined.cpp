// 03 用户自定义转换与 explicit
// C++17；构建验证：Windows + MSVC 19.50（见 README）
#include <cstdio>
#include <string>
#include <type_traits>

// ---- 转换构造函数：单参数构造把其他类型变成自己 ----
struct Seconds {
    Seconds(double s) : value(s) {}           // 非 explicit：允许 double 隐式转成 Seconds
    double value;
};

// ---- explicit 转换构造函数：只允许显式调用 ----
struct PacketId {
    explicit PacketId(int id) : value(id) {}  // explicit：数字不会静默变成协议包号
    int value;
};

// ---- 转换运算符：把自己变成其他类型 ----
class Ratio {
public:
    explicit Ratio(double r) : ratio(r) {}
    operator double() const { return ratio; }              // 非 explicit：Ratio 可隐式转 double
    explicit operator bool() const { return ratio > 0.0; } // explicit：只在布尔语境可用
private:
    double ratio;
};

void start_timer(Seconds s) { std::printf("start_timer(%.1f 秒)\n", s.value); }

// 两级用户自定义转换：B -> A 有转换，C -> B 有转换，但 A -> C 不行（最多一个）
struct A { A(int) {} };
struct B { B(const A&) {} operator A() const { return A{0}; } };
struct C { C(const B&) {} };                 // C 能从 B 构造，但无法从 A 两跳构造

void takes_c(const C&) {}

int main() {
    // ---- 非 explicit 转换构造：实参处隐式转换 ----
    start_timer(1.5);                                        // double -> Seconds
    Seconds s = 2.0;                                         // 拷贝初始化也允许
    std::printf("拷贝初始化 Seconds s = 2.0 -> %.1f\n", s.value);

    // ---- explicit 构造：隐式路径全部封死 ----
    PacketId p(7);                                           // 直接初始化 OK
    PacketId p2 = static_cast<PacketId>(8);                  // 显式转换 OK
    // PacketId bad = 9;                                     // 编译错误
    // void f(PacketId); f(9);                               // 编译错误
    std::printf("PacketId %d、%d 都是显式构造出来的\n", p.value, p2.value);
    static_assert(std::is_constructible<PacketId, int>::value, "显式构造可构");
    static_assert(!std::is_convertible<int, PacketId>::value, "explicit 禁止隐式转换");

    // ---- 转换运算符：对象转 double 的隐式与显式 ----
    Ratio half(0.5);
    double d = half;                                         // operator double 隐式调用
    std::printf("Ratio 隐式转 double = %.2f\n", d);

    // ---- explicit operator bool：语境转换 ----
    if (half) { std::printf("if (half)：布尔语境放行 explicit operator bool\n"); }
    bool explicit_ok = static_cast<bool>(half);              // 显式转换也 OK
    // bool implicit_bad = half;                            // 拷贝初始化：编译错误
    // int arithmetic = half;                               // 想 arithmetic 化？编译错误
    std::printf("static_cast<bool>(half) = %s\n", explicit_ok ? "true" : "false");

    // ---- 用户自定义转换每条链最多一个 ----
    B b = A{1};                                              // A -> B：一个用户转换，OK
    A a = b;                                                 // B -> A：转换运算符，OK
    // takes_c(A{1});                                        // A -> B -> C：两个用户转换，编译错误
    takes_c(C{b});                                           // B -> C：一个，OK
    std::printf("A/B/C 互转验证通过：隐式链最多一个用户自定义转换\n");

    // ---- 布尔语境完整清单（contextual conversion）----
    std::string text = "hi";
    if (!text.empty() && text.size()) {                      // size() 返回 size_t -> bool
        std::printf("if/!/&&/||/?: 是布尔语境；这里 size()=%zu\n", text.size());
    }
    return 0;
}
