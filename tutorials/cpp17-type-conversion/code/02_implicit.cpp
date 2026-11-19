// 02 隐式转换规则：整型提升、算术转换、窄化与符号坑
// C++17；构建验证：Windows + MSVC 19.50（见 README）
#include <cstdio>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

int main() {
    // ---- 整型提升：小于 int 的类型先变 int，值保持不变 ----
    unsigned char uc = 250;
    auto sum = uc + uc;                       // 提升为 int 运算：500，不回绕
    static_assert(std::is_same<decltype(sum), int>::value, "提升后是 int");
    std::printf("unsigned char 250 + 250 = %d，类型 int（提升保住了值）\n", sum);

    // ---- usual arithmetic conversions：-1 < 1u 是 false ----
    // 同秩时 signed 向 unsigned 看齐：-1 转成 0xFFFFFFFF
    bool cmp = (-1 < 1u);
    std::printf("-1 < 1u 结果 = %s（-1 被转成 %u）\n",
                cmp ? "true" : "false", static_cast<unsigned>(-1));
    static_assert(!(-1 < 1u), "同秩比较 signed 转 unsigned");

    // ---- 混合符号运算：unsigned int - 1 回绕 ----
    unsigned int u = 0;
    std::printf("unsigned 0u - 1 = %u（模 2^32 回绕）\n", u - 1);

    // ---- 窄化转换：拷贝初始化放行，列表初始化拦下 ----
    double d = 3.9;
#pragma warning(push)
#pragma warning(disable : 4244)  // 教学演示：隐式窄化在 /W4 下正是 C4244 警告
    int i = d;                                // 向零截断得 3
#pragma warning(pop)
    // int j{d};                              // 列表初始化禁止窄化：编译错误
    std::printf("double 3.9 -> int %d（向零截断）；int{3.9} 无法编译\n", i);

    // ---- 浮点转整型超出范围是 UB；整型转浮点可能丢精度 ----
    long long big = 16777217;                 // 2^24 + 1
    float f = static_cast<float>(big);        // float 只有 24 位有效尾数
    std::printf("16777217 转 float 再回来 = %.0f（精度丢失）\n",
                static_cast<double>(f));

    // ---- 指针转换：数组退化 + void* 往返 ----
    int arr[5] = {10, 20, 30, 40, 50};
    int* p = arr;                             // 数组到指针退化
    void* v = p;                              // 任意对象指针 -> void*（隐式）
    int* back = static_cast<int*>(v);         // void* -> T* 需要 static_cast
    std::printf("数组退化 arr[2]=%d；void* 往返后 arr[2]=%d\n", p[2], back[2]);

    // ---- 派生类指针到基类指针：隐式上行 ----
    struct Base { int b = 1; };
    struct Derived : Base { int d = 2; };
    Derived der;
    Base* bp = &der;                          // 上行转换隐式、安全
    std::printf("上行转换后 bp->b = %d\n", bp->b);

    // ---- 日常头号坑：size_t 与 int 比较 ----
    std::vector<int> vec;
    int n = -1;
    // vec.size() 是无符号 size_t：n 被转成巨大的无符号数
    std::printf("vec.empty()=%s 时 size()=%zu，n=-1 与 size() 比较结果 = %s\n",
                vec.empty() ? "true" : "false", vec.size(),
                (n < static_cast<int>(vec.size())) ? "true" : "false");
    std::printf("直接写 n < vec.size() 得到 %s（-1 被转成巨大无符号数）\n",
                (n < vec.size()) ? "true" : "false");

    // ---- 附加：bool 转换（指针/数值 -> bool）----
    int* np = nullptr;
    std::printf("nullptr -> bool = %s，&sum -> bool = %s\n",
                static_cast<bool>(np) ? "true" : "false",
                static_cast<bool>(&sum) ? "true" : "false");
    return 0;
}
