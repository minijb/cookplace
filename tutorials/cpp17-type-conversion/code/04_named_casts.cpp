// 04 命名 cast 四剑客与 C 风格 cast
// C++17；构建验证：Windows + MSVC 19.50（见 README）
#include <cstdio>
#include <cstdint>
#include <cstring>

// ---- static_cast 的地盘：编译期语义明确的转换 ----
enum class GameState : int { Menu = 0, Playing = 1, Paused = 2 };

struct Base { int b = 1; virtual ~Base() = default; };
struct Derived : Base { int d = 2; };

// 假想的 C 接口：参数没写 const（但实际只读），逼你交出非 const 指针
std::size_t legacy_string_length(char* s) { return std::strlen(s); }

int main() {
    // ---- static_cast：枚举 <-> 整数、浮点 <-> 整数 ----
    GameState st = GameState::Playing;
    int raw = static_cast<int>(st);
    GameState back = static_cast<GameState>(raw);
    int truncated = static_cast<int>(3.99);
    std::printf("static_cast：枚举 %d 往返 OK，3.99 截断为 %d\n",
                raw, truncated);
    std::printf("GameState back = %d\n", static_cast<int>(back));

    // ---- static_cast 下行转换：编译器信任你，类型错了是 UB ----
    Derived der;
    Base* bp = &der;
    Derived* dp = static_cast<Derived*>(bp);      // 正确的下行转换，编译期完成
    std::printf("static_cast 下行：dp->d = %d（无运行期检查）\n", dp->d);
    Base other;
    // Derived* wrong = static_cast<Derived*>(&other);  // 编译能过！
    // std::printf("%d\n", wrong->d);                  // 但运行期读 d 是 UB：越界内存

    // ---- dynamic_cast：多态下行转换，带运行期检查 ----
    Base* maybe_derived = &der;
    Base* really_base = &other;
    if (auto* ok = dynamic_cast<Derived*>(maybe_derived)) {
        std::printf("dynamic_cast 成功：ok->d = %d\n", ok->d);
    }
    auto* failed = dynamic_cast<Derived*>(really_base);
    std::printf("dynamic_cast 失败返回 %s 指针\n", failed == nullptr ? "空" : "非空");

    // ---- const_cast：去 const 只为适配旧接口；改动真正的 const 对象是 UB ----
    const char* msg = "hello legacy api";
    std::size_t len = legacy_string_length(const_cast<char*>(msg)); // 合法：API 实际只读
    // char* mutable_view = const_cast<char*>(msg); mutable_view[0] = 'H';  // UB！
    std::printf("const_cast 适配只读旧接口：len = %zu\n", len);

    // ---- reinterpret_cast：几乎不给你任何保证 ----
    int value = 42;
    std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(&value); // 指针 <-> 整数：合法
    int* same = reinterpret_cast<int*>(addr);
    std::printf("reinterpret_cast 指针往返：addr = 0x%zx，值还原 = %d\n", addr, *same);
    // float f = 1.0f; int bits = *reinterpret_cast<int*>(&f);  // 违反严格别名：UB！第 6 章讲

    // ---- C 风格 cast：一把能撬开所有门的万能钥匙 ----
    class PrivBase { public: int x = 10; };
    class PrivDerived : private PrivBase {                       // 私有继承
    public: int y = 20;
    };
    PrivDerived pd;
    // PrivBase* pb1 = static_cast<PrivBase*>(&pd);              // 编译错误：继承是 private
    PrivBase* pb2 = (PrivBase*)&pd;                              // C 风格 cast 静默无视 private！
    std::printf("C 风格 cast 绕过 private 继承：pb2->x = %d\n", pb2->x);

    // struct Unrelated { int u = 1; };
    // Unrelated* ur = (Unrelated*)&value;  // C 风格：reinterpret_cast 化身，编译照过
    // static_cast<Unrelated*>(&value);     // 命名 cast：编译报错，拦住你
    std::printf("C 风格 cast = const/static/reinterpret 的按序尝试组合，出错无声\n");
    return 0;
}
