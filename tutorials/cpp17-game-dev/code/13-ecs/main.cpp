#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
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

// ---------------------------------------------------------------------------
// 1. 实体：索引 + 代数（generation）的轻量句柄
// ---------------------------------------------------------------------------

constexpr std::uint32_t kNullIndex = ~std::uint32_t{0};

struct Entity {
    std::uint32_t index = kNullIndex;
    std::uint32_t generation = 0;
    bool valid() const { return index != kNullIndex; }
};

// ---------------------------------------------------------------------------
// 2. 组件池：sparse set —— sparse 存映射，dense 存数据
// ---------------------------------------------------------------------------

template <typename Component>
class Pool {
public:
    void grow(std::uint32_t index) {
        if (sparse_.size() <= index) {
            sparse_.resize(static_cast<std::size_t>(index) + 1, kNullIndex);
        }
    }

    void add(Entity entity, Component component) {
        grow(entity.index);
        if (contains(entity)) {
            dense_[sparse_[entity.index]] = std::move(component);
            return;
        }
        sparse_[entity.index] = static_cast<std::uint32_t>(dense_.size());
        dense_.push_back(std::move(component));
        entities_.push_back(entity);
    }

    void remove(Entity entity) {
        if (!contains(entity)) {
            return;
        }
        // swap-remove：把末尾元素搬到空洞，dense 数组保持紧凑。
        const std::uint32_t dense_i = sparse_[entity.index];
        const std::uint32_t last_i =
            static_cast<std::uint32_t>(dense_.size() - 1);
        if (dense_i != last_i) {
            dense_[dense_i] = std::move(dense_[last_i]);
            entities_[dense_i] = entities_[last_i];
            sparse_[entities_[dense_i].index] = dense_i;
        }
        dense_.pop_back();
        entities_.pop_back();
        sparse_[entity.index] = kNullIndex;
    }

    Component* try_get(Entity entity) {
        if (!contains(entity)) {
            return nullptr;
        }
        return &dense_[sparse_[entity.index]];
    }

    bool contains(Entity entity) const {
        return entity.index < sparse_.size() &&
               sparse_[entity.index] != kNullIndex;
    }

    std::vector<Component>& dense() { return dense_; }
    const std::vector<Entity>& entities() const { return entities_; }
    std::size_t size() const { return dense_.size(); }

private:
    std::vector<std::uint32_t> sparse_; // 实体索引 → dense 下标（或 kNull）
    std::vector<Component> dense_;      // 紧凑组件数组
    std::vector<Entity> entities_;      // 与 dense 平行的属主实体
};

// ---------------------------------------------------------------------------
// 3. 组件类型：纯数据，没有行为
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// 4. Registry：实体生命周期 + 三张组件池
// ---------------------------------------------------------------------------

class Registry {
public:
    Entity create() {
        if (!free_indices_.empty()) {
            // 复用槽位：代数已在 destroy 时递增，旧句柄自动失效。
            const std::uint32_t index = free_indices_.back();
            free_indices_.pop_back();
            return Entity{index, generations_[index]};
        }
        const std::uint32_t index =
            static_cast<std::uint32_t>(generations_.size());
        generations_.push_back(0);
        return Entity{index, 0};
    }

    void destroy(Entity entity) {
        if (!alive(entity)) {
            return;
        }
        positions_.remove(entity);
        velocities_.remove(entity);
        healths_.remove(entity);
        ++generations_[entity.index]; // 递增代数：旧 Entity 句柄失效
        free_indices_.push_back(entity.index);
    }

    bool alive(Entity entity) const {
        return entity.valid() && entity.index < generations_.size() &&
               generations_[entity.index] == entity.generation;
    }

    Pool<Position>& positions() { return positions_; }
    Pool<Velocity>& velocities() { return velocities_; }
    Pool<Health>& healths() { return healths_; }

private:
    std::vector<std::uint32_t> generations_;
    std::vector<std::uint32_t> free_indices_;
    Pool<Position> positions_;
    Pool<Velocity> velocities_;
    Pool<Health> healths_;
};

// ---------------------------------------------------------------------------
// 5. View：按最小集合遍历，逐个校验其余组件的存在
// ---------------------------------------------------------------------------

template <typename... Components>
class View;

template <typename Head, typename... Rest>
class View<Head, Rest...> {
public:
    View(Pool<Head>& head, Pool<Rest>&... rest) : head_(head), rest_(rest...) {}

    template <typename Fn>
    void each(Fn&& fn) {
        // 以 Head 池（最小的）为驱动；dense 数组连续，预取友好。
        const std::vector<Entity>& owners = head_.entities();
        for (std::size_t i = 0; i < owners.size(); ++i) {
            Entity entity = owners[i];
            Head& head_component = head_.dense()[i];
            iterate_rest(entity, head_component, fn,
                         std::index_sequence_for<Rest...>{});
        }
    }

private:
    template <typename Fn, std::size_t... Is>
    void iterate_rest(Entity entity, Head& head_component, Fn& fn,
                      std::index_sequence<Is...>) {
        // 元组里每个池都查一遍；缺组件的实体直接跳过。
        std::tuple<Pool<Rest>&...>& pools = rest_;
        if ((std::get<Is>(pools).contains(entity) && ...)) {
            fn(entity, head_component,
               *std::get<Is>(pools).try_get(entity)...);
        }
    }

    Pool<Head>& head_;
    std::tuple<Pool<Rest>&...> rest_;
};

template <typename... Components>
View<Components...> view(Pool<Components>&... pools) {
    return View<Components...>(pools...);
}

// ---------------------------------------------------------------------------
// 6. 系统：批量处理匹配组件的普通函数
// ---------------------------------------------------------------------------

void movement_system(Registry& registry, float dt) {
    view(registry.positions(), registry.velocities())
        .each([dt](Entity, Position& p, const Velocity& v) {
            p.x += v.dx * dt;
            p.y += v.dy * dt;
        });
}

// 系统 iterated 途中销毁实体会让 swap-remove 打乱正在遍历的数组；
// 正确做法是先收集，遍历结束后统一销毁。
void damage_system(Registry& registry, std::vector<Entity>& dead_out) {
    view(registry.healths(), registry.positions())
        .each([&dead_out](Entity e, const Health& h, const Position&) {
            if (h.hp <= 0) {
                dead_out.push_back(e);
            }
        });
}

// ---------------------------------------------------------------------------
// 7. 演示：创建、迭代、销毁与句柄失效
// ---------------------------------------------------------------------------

void ecs_demo() {
    Registry registry;

    Entity movers[8];
    for (Entity& e : movers) {
        e = registry.create();
        registry.positions().add(e, Position{});
        registry.velocities().add(e, Velocity{1.0f, 2.0f});
        registry.healths().add(e, Health{100});
    }
    // 只有位置、没有速度的实体：view<Position, Velocity> 会跳过它。
    Entity static_entity = registry.create();
    registry.positions().add(static_entity, Position{5.0f, 5.0f});

    movement_system(registry, 1.0f);
    REQUIRE(registry.positions().try_get(movers[0])->x == 1.0f);
    REQUIRE(registry.positions().try_get(static_entity)->x == 5.0f);

    // 击杀一个实体并在遍历后销毁。
    registry.healths().dense()[0].hp = 0;
    std::vector<Entity> dead;
    damage_system(registry, dead);
    REQUIRE(dead.size() == 1);
    for (Entity e : dead) {
        registry.destroy(e);
    }
    REQUIRE(!registry.alive(dead[0]));

    // 复用槽位：新实体拿到同一 index 但更高的 generation。
    Entity recycled = registry.create();
    REQUIRE(recycled.index == dead[0].index);
    REQUIRE(recycled.generation != dead[0].generation);
    REQUIRE(!registry.alive(dead[0])); // 旧句柄仍然失效
    REQUIRE(registry.alive(recycled));
}

void ecs_scale() {
    constexpr int kEntities = 100000;
    constexpr int kFrames = 60;
    Registry registry;
    for (int i = 0; i < kEntities; ++i) {
        Entity e = registry.create();
        registry.positions().add(e, Position{});
        if (i % 2 == 0) {
            registry.velocities().add(e, Velocity{0.1f, 0.1f});
        }
    }

    const auto start = std::chrono::steady_clock::now();
    for (int frame = 0; frame < kFrames; ++frame) {
        movement_system(registry, 1.0f / 60.0f);
    }
    const auto elapsed = std::chrono::steady_clock::now() - start;

    std::cout << "moved " << kEntities << " entities x " << kFrames
              << " frames in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed)
                     .count()
              << " ms\n";

    // 偶数实体移动了 1 秒：0.1 * 60 = 6；奇数实体不动。
    std::size_t moved = 0;
    view(registry.positions(), registry.velocities())
        .each([&moved](Entity, const Position& p, const Velocity&) {
            // 速度 0.1 × 1 秒 = 0.1；浮点累加用容差断言。
            constexpr float kEps = 1e-4f;
            REQUIRE(std::fabs(p.x - 0.1f) < kEps &&
                    std::fabs(p.y - 0.1f) < kEps);
            ++moved;
        });
    REQUIRE(moved == static_cast<std::size_t>(kEntities / 2));
}

int main() {
    ecs_demo();
    ecs_scale();
    std::cout << "ch04_ecs: all checks passed\n";
    return 0;
}
