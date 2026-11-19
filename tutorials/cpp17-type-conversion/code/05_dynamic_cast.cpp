// 05 dynamic_cast 深入：下行/跨类转换、this 指针调整与成本
// C++17；构建验证：Windows + MSVC 19.50（见 README）
#include <cstdio>
#include <chrono>
#include <typeinfo>

// Entity 与 Weapon 是两个独立基类，Character 同时继承两者
struct Entity { virtual ~Entity() = default; int hp = 100; };
struct Weapon { virtual ~Weapon() = default; int damage = 7; };
struct Character : Entity, Weapon { const char* name = "hero"; };

int main() {
    Character hero;

    // ---- 同一对象，两个基类子对象地址不同（this 指针调整的根源）----
    Entity* ep = &hero;              // 编译期就知道偏移：Entity 子对象在开头
    Weapon* wp = &hero;              // 编译期插入固定偏移调整
    std::printf("hero 对象地址 %p，作为 Entity* %p，作为 Weapon* %p\n",
                static_cast<void*>(&hero), static_cast<void*>(ep),
                static_cast<void*>(wp));
    std::printf("Entity* 与 Weapon* 差值 = %td 字节（多继承的 this 调整）\n",
                reinterpret_cast<char*>(wp) - reinterpret_cast<char*>(ep));

    // ---- 下行转换：多态类型才有 dynamic_cast 资格 ----
    Entity* unknown = &hero;
    if (auto* c = dynamic_cast<Character*>(unknown)) {
        std::printf("下行成功：%s（运行期查 RTTI 元数据）\n", c->name);
    }

    // ---- 跨类转换：Entity* -> Weapon*，编译器不知道偏移，运行期查表 ----
    if (auto* w = dynamic_cast<Weapon*>(ep)) {
        std::printf("跨类成功：damage = %d（同一 Character 的另一个基类子对象）\n", w->damage);
    }
    Entity unrelated;
    std::printf("对非 Character 实体跨类：dynamic_cast<Weapon*> = %s\n",
                dynamic_cast<Weapon*>(&unrelated) == nullptr ? "nullptr" : "非空");

    // ---- 失败行为：指针返 nullptr，引用抛 std::bad_cast ----
    try {
        dynamic_cast<Weapon&>(unrelated);                // 引用失败无处放 nullptr
    } catch (const std::bad_cast&) {
        std::printf("引用形式失败：抛 std::bad_cast\n");
    }

    // ---- dynamic_cast<void*>：拿到最派生对象地址 ----
    std::printf("dynamic_cast<void*>(ep) = %p（== 最派生地址 %p：%s）\n",
                dynamic_cast<void*>(ep), static_cast<void*>(&hero),
                dynamic_cast<void*>(ep) == static_cast<void*>(&hero) ? "是" : "否");

    // ---- 成本对比：dynamic_cast 走元数据，static_cast 编译期完成 ----
    constexpr int N = 10000000;
    volatile std::ptrdiff_t sink = 0;                    // 防止优化掉整个循环
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) sink += dynamic_cast<Character*>(ep) != nullptr;
    auto t1 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) sink += static_cast<Character*>(unknown) != nullptr;
    auto t2 = std::chrono::steady_clock::now();
    auto dyn_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    auto sta_us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    std::printf("%d 次：dynamic_cast %lld us，static_cast %lld us（本机量级参考）\n",
                N, static_cast<long long>(dyn_us), static_cast<long long>(sta_us));
    std::printf("层深/兄弟越多越慢；实现间差距可达数倍（libc++ 曾比 libstdc++ 慢 8~30 倍）\n");
    std::printf("sink=%td\n", sink);
    return 0;
}
