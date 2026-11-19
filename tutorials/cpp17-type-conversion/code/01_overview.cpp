// 01 转换全景：类型转换发生在哪里
// C++17；构建验证：Windows + MSVC 19.50（见 README）
#include <cstdio>
#include <string>
#include <type_traits>

// 隐式转换构造函数：double 可以静默变成 Meters
struct Meters {
    explicit Meters(double m) : value(m) {}   // explicit：禁止隐式转换
    double value;
};

// 接受 Meters 的函数：如果不 explicit，这里就能直接传 double
void print_length(double m) {                 // 形参类型 = 转换目标
    std::printf("print_length 收到 %.2f 米\n", m);
}

int main() {
    // ---- 场景 1：初始化（拷贝初始化 vs 直接初始化）----
#pragma warning(push)
#pragma warning(disable : 4244)  // 教学演示：隐式窄化在 /W4 下正是 C4244 警告
    int a = 3.9;                 // 隐式窄化：拷贝初始化允许，值为 3（向零截断）
#pragma warning(pop)
    // int b{3.9};               // 列表初始化禁止窄化：编译错误
    int c{static_cast<int>(3.9)};// 显式转换后，列表初始化放行
    std::printf("窄化拷贝初始化 int a = 3.9 -> %d；显式转换后 {static_cast} -> %d\n", a, c);

    // ---- 场景 2：函数实参 ----
    print_length(42);            // int -> double：标准转换，宽化，安全

    // ---- 场景 3：运算符操作数（算术转换）----
    unsigned char uc = 250;      // 先提升为 int 再运算，不会回绕
    int sum = uc + uc;
    std::printf("unsigned char 250 + 250 = %d（类型为 int，不是 unsigned char）\n", sum);
    static_assert(std::is_same<decltype(uc + uc), int>::value, "整型提升到 int");

    // ---- 场景 4：return / throw / catch ----
    auto to_int = [](double d) -> int {
#pragma warning(push)
#pragma warning(disable : 4244)  // 教学演示：return 语句处的隐式窄化
        return d;             // return 语句处的转换
#pragma warning(pop)
    };  // 实际宽 /W4 会报 C4244
    std::printf("return 处转换 %.2f -> %d\n", 7.9, to_int(7.9));

    // ---- 场景 5：explicit 阻止隐式转换 ----
    Meters m(5.0);                            // 直接初始化：explicit 构造 OK
    // Meters bad = 5.0;                      // 拷贝初始化：explicit 构造被禁止，编译错误
    // print_meters(5.0);                     // 同理，不能把 double 静默变成 Meters
    std::printf("explicit 构造只能直接初始化或 static_cast：%.1f 米\n", m.value);
    Meters m2 = static_cast<Meters>(6.5);     // 显式调用 explicit 构造
    std::printf("static_cast<Meters>(6.5) -> %.1f 米\n", m2.value);

    // ---- 场景 6：转换大部分发生在编译期，只有 dynamic_cast 等少数在运行期 ----
    std::printf("sizeof 阶段类型已定：%zu 字节的 int 转换规则编译期就确定\n", sizeof(int));
    return 0;
}
