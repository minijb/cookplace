// 07 UE Cast 专题（模拟实现）：反射式转换与 dynamic_cast 的区别
// C++17；构建验证：Windows + MSVC 19.50（见 README）
// 说明：UE 构建时关闭 C++ RTTI（/GR-），dynamic_cast 不可用；
// 本文件用最小反射注册表「模拟」UE Cast<T> 的语义，非 UE 源码。
#include <cstdio>
#include <stdexcept>
#include <string>

// ---- 极简反射：每个类型登记自己的 UClass（名字 + 父类）----
struct UClass {
    const char* name;
    const UClass* super;              // 父类，nullptr 表示根（相当于 UObject）
    bool is_child_of(const UClass& other) const {
        for (const UClass* c = this; c; c = c->super) {
            if (c == &other) return true;   // 沿继承链上溯：UE 里即 IsChildOf
        }
        return false;
    }
};

// 模拟 UObject：所有参与者都带 ClassPrivate
struct UObject {
    const UClass* ClassPrivate = nullptr;
    virtual ~UObject() = default;
};

// ---- 各类型登记 UClass 并提供 StaticClass() ----
struct UClass ActorClass{"AActor", nullptr};
struct UClass PawnClass{"APawn", &ActorClass};
struct UClass CharacterClass{"ACharacter", &PawnClass};
struct UClass ComponentClass{"UActorComponent", nullptr};   // 与 Actor 树无关

struct AActor : UObject {
    static const UClass* StaticClass() { return &ActorClass; }
};
struct APawn : AActor {
    static const UClass* StaticClass() { return &PawnClass; }
};
struct ACharacter : APawn {
    static const UClass* StaticClass() { return &CharacterClass; }
    int jump_count = 0;
};
struct UActorComponent : UObject {
    static const UClass* StaticClass() { return &ComponentClass; }
    int tick_count = 0;
};

// ---- 模拟 UE 的 Cast<T>：先反射查类型，再调整指针 ----
template <typename To, typename From>
To* Cast(From* src) {
    if (src == nullptr) return nullptr;
    if (!src->ClassPrivate->is_child_of(*To::StaticClass())) return nullptr;  // 反射判定
    // 反射通过后用 reinterpret_cast 落实指针转换：UObject 继承链是单继承，
    // 子对象偏移恒为 0，这正是 UE 允许这样写的设计前提；static_cast 在
    // 无关类树之间根本编译不过，所以这里不能用它。
    return reinterpret_cast<To*>(src);
}

// 模拟 CastChecked：失败即中断（UE 中是 assert/crash，模拟版抛异常便于测试）
template <typename To, typename From>
To* CastChecked(From* src) {
    To* result = Cast<To>(src);
    if (!result) throw std::runtime_error(std::string("CastChecked 失败：对象不是 ") +
                                          To::StaticClass()->name);
    return result;
}

int main() {
    ACharacter character;
    UActorComponent component;
    character.ClassPrivate = &CharacterClass;
    component.ClassPrivate = &ComponentClass;

    // ---- 沿继承链 Cast：成功 ----
    if (auto* pawn = Cast<APawn>(&character)) {
        std::printf("Cast<APawn>(ACharacter*) 成功：%s 沿 super 链找到 %s\n",
                    pawn->ClassPrivate->name, PawnClass.name);
    }

    // ---- 无关类树之间的 Cast：失败返回 nullptr，绝不崩溃 ----
    auto* no_comp = Cast<UActorComponent>(&character);
    std::printf("Cast<UActorComponent>(ACharacter*) = %s（反射判定不属于该类树）\n",
                no_comp == nullptr ? "nullptr" : "非空");
    auto* no_actor = Cast<AActor>(&component);
    std::printf("Cast<AActor>(UActorComponent*) = %s\n",
                no_actor == nullptr ? "nullptr" : "非空");

    // ---- 成功后安全访问子类成员 ----
    if (auto* c = Cast<ACharacter>(&character)) {
        c->jump_count = 3;                       // 类型确认后才能碰子类成员
        std::printf("Cast<ACharacter> 成功：jump_count = %d\n", c->jump_count);
    }

    // ---- CastChecked：失败的类型假设必须立刻暴露 ----
    try {
        CastChecked<AActor>(&character);         // ACharacter 是 AActor 子类：通过
        CastChecked<ACharacter>(&component);     // 组件不是角色：抛出
    } catch (const std::runtime_error& e) {
        std::printf("CastChecked 拦截错误假设：%s\n", e.what());
    }

    // ---- 与 dynamic_cast 的差异（UE 场景）----
    std::printf("UE 关闭 RTTI：dynamic_cast 编译不过；UObject 用反射 UClass 判型，\n");
    std::printf("非编辑器构建 Cast 判型近乎常数时间，失败语义同样是返回 nullptr\n");
    return 0;
}
