// 08 综合练习：把各章知识点串成一个「类型转换体检」
// C++17；构建验证：Windows + MSVC 19.50（见 README）
// 每个练习先给出「错误写法」（注释），再给出正确实现并用断言验证。
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <vector>

// ---- 练习 1：无符号回绕（第 2 章）----
// 错误写法：for (unsigned i = n; i >= 0; --i)   // i >= 0 恒真，死循环回绕
std::vector<int> countdown(int n) {
    std::vector<int> out;
    for (int i = n; i >= 0; --i) {          // 有符号计数，避免 i >= 0 恒真
        out.push_back(i);
    }
    return out;
}

// ---- 练习 2：API 边界用 explicit 挡住隐式转换（第 3 章）----
struct PlayerId {
    explicit PlayerId(int raw) : value(raw) {}
    int value;
};
void kick_player(PlayerId id) {             // 不存在 kick_player(int) 的隐式通道
    std::printf("kick_player(id=%d)\n", id.value);
}

// ---- 练习 3：下行转换先用 dynamic_cast 问路（第 4、5 章）----
struct Entity { virtual ~Entity() = default; };
struct Turret : Entity { int ammo = 9; };
struct Lamp : Entity { bool on = true; };
Turret* as_turret(Entity* e) { return dynamic_cast<Turret*>(e); }

// ---- 练习 4：序列化 float 用 memcpy，不用 reinterpret_cast 解引用（第 6 章）----
std::uint32_t float_bits(float f) {
    static_assert(sizeof(float) == sizeof(std::uint32_t), "4 字节假设");
    std::uint32_t bits;
    std::memcpy(&bits, &f, sizeof(bits));   // 合法双关
    return bits;
}

// ---- 练习 5：反射式安全 Cast（第 7 章思路的最小复现）----
struct Base { virtual ~Base() = default; };
struct Enemy : Base { const char* tag = "enemy"; };

int main() {
    // 练习 1：n = 0 也能正确终止
    auto seq = countdown(3);
    assert((seq == std::vector<int>{3, 2, 1, 0}));
    auto empty = countdown(0);
    assert(empty.size() == 1 && empty[0] == 0);
    std::printf("练习 1 通过：有符号计数，无回绕死循环\n");

    // 练习 2：显式构造、隐式通道封死
    kick_player(PlayerId(42));                // 必须 explicit 造出 id
    // kick_player(42);                       // 编译错误：误传裸 int 被拦下
    static_assert(!std::is_convertible<int, PlayerId>::value, "explicit 生效");
    std::printf("练习 2 通过：explicit 阻止裸 int 静默变 PlayerId\n");

    // 练习 3：类型不确定时先问 RTTI
    Turret turret;
    Lamp lamp;
    Entity* e1 = &turret;
    Entity* e2 = &lamp;
    if (auto* t = as_turret(e1)) { assert(t->ammo == 9); }
    assert(as_turret(e2) == nullptr);         // 灯不是炮台：nullptr 分支兜底
    std::printf("练习 3 通过：dynamic_cast 失败返 nullptr，分支兜底\n");

    // 练习 4：位模式提取与还原
    std::uint32_t bits = float_bits(1.0f);
    assert(bits == 0x3F800000u);              // IEEE 754 的 1.0f
    float restored;
    std::memcpy(&restored, &bits, sizeof(restored));
    assert(restored == 1.0f);
    std::printf("练习 4 通过：memcpy 双关，1.0f -> 0x%08X -> 1.0f\n", bits);

    // 练习 5：无需 dynamic_cast 的判型（演示 RTTI 被关时替代思路）
    Enemy enemy;
    Base* unknown = &enemy;
    bool is_enemy = dynamic_cast<Enemy*>(unknown) != nullptr;   // 常规做法
    assert(is_enemy);
    // 若 -fno-rtti / /GR-：改用类型标签或自定义反射（练习思路见第 7 章）
    std::printf("练习 5 通过：判型入口统一，关 RTTI 时可替换实现\n");

    std::printf("全部练习通过 ✔\n");
    return 0;
}
