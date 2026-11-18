// 同一个模拟，用 vendored EnTT v3.13.2（sparse set 工业实现）重写一遍，
// 便于对照手写迷你 ECS 与真实库的 API 与行为。

#include <entt/entt.hpp>

#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>

#define REQUIRE(cond)                                                          \
    do {                                                                       \
        if (!(cond)) {                                                         \
            std::cerr << "REQUIRE failed: " #cond " at line " << __LINE__      \
                      << '\n';                                                 \
            std::abort();                                                      \
        }                                                                      \
    } while (0)

struct Position {
    float x = 0.0f;
    float y = 0.0f;
};
struct Velocity {
    float dx = 0.0f;
    float dy = 0.0f;
};
struct Health {
    int hp = 0;
};

int main() {
    entt::registry registry;
    std::vector<entt::entity> movers;
    movers.reserve(8);
    for (int i = 0; i < 8; ++i) {
        const entt::entity e = registry.create();
        registry.emplace<Position>(e);
        registry.emplace<Velocity>(e, 1.0f, 2.0f);
        registry.emplace<Health>(e, 100);
        movers.push_back(e);
    }
    const entt::entity static_entity = registry.create();
    registry.emplace<Position>(static_entity, 5.0f, 5.0f);

    // view 按最小集合迭代；each 直接拿到组件引用，没有实体也能用。
    registry.view<Position, Velocity>().each(
        [](Position& p, const Velocity& v) { p.x += v.dx; p.y += v.dy; });
    REQUIRE(registry.get<Position>(movers[0]).x == 1.0f);
    REQUIRE(registry.get<Position>(static_entity).x == 5.0f);

    // 先收集再销毁：与手写版相同的遍历纪律（销毁会 swap-remove 打乱数组）。
    registry.patch<Health>(movers[0], [](Health& h) { h.hp = 0; });
    std::vector<entt::entity> dead;
    registry.view<const Health>().each(
        [&dead](const entt::entity e, const Health& h) {
            if (h.hp <= 0) {
                dead.push_back(e);
            }
        });
    REQUIRE(dead.size() == 1);
    for (const entt::entity e : dead) {
        registry.destroy(e);
    }
    REQUIRE(!registry.valid(dead[0])); // 销毁后实体句柄失效

    // EnTT 实体同样是「索引 + 代数」句柄：普通 create() 回收同一槽位
    // 并把版本号 +1，旧句柄不会复活。
    const entt::entity recycled = registry.create();
    REQUIRE(recycled != movers[0]); // 版本不同，句柄不相等
    REQUIRE(entt::to_entity(recycled) == entt::to_entity(movers[0]));
    REQUIRE(!registry.valid(movers[0]));

    // ⚠️ 不要用 create(hint) 传过期句柄来「重建」：实测 v3.13 会把过期
    // 句柄原样返回（valid 再次变 true）。它面向快照恢复等场景，误用会
    // 悄悄复活旧句柄。

    // 规模演示：10 万实体 × 60 帧移动（独立 registry，避免与演示实体混算）。
    constexpr int kEntities = 100000;
    entt::registry world;
    for (int i = 0; i < kEntities; ++i) {
        const entt::entity e = world.create();
        world.emplace<Position>(e);
        if (i % 2 == 0) {
            world.emplace<Velocity>(e, 0.1f, 0.1f);
        }
    }
    const auto start = std::chrono::steady_clock::now();
    for (int frame = 0; frame < 60; ++frame) {
        world.view<Position, Velocity>().each(
            [](Position& p, const Velocity& v) {
                p.x += v.dx / 60.0f;
                p.y += v.dy / 60.0f;
            });
    }
    const auto elapsed = std::chrono::steady_clock::now() - start;
    std::cout << "EnTT moved " << kEntities << " entities x 60 frames in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed)
                     .count()
              << " ms\n";

    std::size_t moved = 0;
    world.view<Position, const Velocity>().each(
        [&moved](const Position& p, const Velocity&) {
            // 速度 0.1 × 1 秒 = 0.1；浮点累加用容差断言。
            constexpr float kEps = 1e-4f;
            REQUIRE(std::fabs(p.x - 0.1f) < kEps &&
                    std::fabs(p.y - 0.1f) < kEps);
            ++moved;
        });
    REQUIRE(moved == static_cast<std::size_t>(kEntities / 2));

    std::cout << "ch04_entt_demo: all checks passed\n";
    return 0;
}
