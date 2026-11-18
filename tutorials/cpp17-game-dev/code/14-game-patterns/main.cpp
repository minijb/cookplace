#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <queue>
#include <random>
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
// 1. Update Method：每个对象自己知道每帧做什么
// ---------------------------------------------------------------------------

struct GameObject {
    virtual ~GameObject() = default;
    virtual void update(double dt) = 0;
};

struct Enemy : GameObject {
    explicit Enemy(double start_x) : x(start_x) {}
    void update(double dt) override { x += speed * dt; }
    double x = 0.0;
    double speed = 5.0;
};

struct FloatingText : GameObject {
    explicit FloatingText(double start_life) : life(start_life) {}
    void update(double dt) override { life -= dt; }
    double life = 0.0;
};

void update_method_demo() {
    std::vector<std::unique_ptr<GameObject>> world;
    world.push_back(std::make_unique<Enemy>(0.0));
    world.push_back(std::make_unique<FloatingText>(0.4));

    const std::size_t kFrames = 30;
    for (std::size_t i = 0; i < kFrames; ++i) {
        for (auto& object : world) {
            object->update(1.0 / 60.0); // 主循环只管统一驱动
        }
    }

    auto* enemy = static_cast<Enemy*>(world[0].get());
    REQUIRE(enemy->x == 5.0 * 30.0 / 60.0); // 走了 0.5 秒
    auto* text = static_cast<FloatingText*>(world[1].get());
    REQUIRE(text->life < 0.0); // 0.4 秒寿命在 0.5 秒后耗尽（销毁策略略）
}

// ---------------------------------------------------------------------------
// 2. Observer：事件发生后通知订阅者；EventQueue：延迟到安全时机处理
// ---------------------------------------------------------------------------

enum class GameEvent { EntityFell, EntityDied };

class AchievementSystem {
public:
    void on_event(GameEvent event) {
        if (event == GameEvent::EntityFell) {
            fell_unlocked_ = true;
        }
    }
    bool fell_unlocked() const { return fell_unlocked_; }

private:
    bool fell_unlocked_ = false;
};

void observer_and_event_queue_demo() {
    AchievementSystem achievements;
    std::queue<GameEvent> event_queue;

    // 物理代码只负责把事件入队：不知道也不关心谁在听。
    event_queue.push(GameEvent::EntityFell);

    // 立即分发是 Observer 的直接形态；入队不处理则保证「事件产生的
    // 副作用」延迟到我们选的安全时机（帧末），避免遍历中改世界。
    REQUIRE(!achievements.fell_unlocked()); // 还没 flush，没有副作用

    while (!event_queue.empty()) {
        achievements.on_event(event_queue.front());
        event_queue.pop();
    }
    REQUIRE(achievements.fell_unlocked()); // flush 后解锁

    // 权衡：立即通知响应最快，但订阅者可能反过来改场景，时序难推理；
    // 事件队列牺牲一帧延迟换取可预测的分发时机。
}

// ---------------------------------------------------------------------------
// 3. Object Pool：热路径零分配，槽位循环复用
// ---------------------------------------------------------------------------

struct Particle {
    double x = 0.0;
    double y = 0.0;
    double life = 0.0;
    bool active = false;
};

class ParticlePool {
public:
    explicit ParticlePool(std::size_t capacity)
        : slots_(capacity) {} // 一次分配，之后 spawn/kill 不再碰堆

    Particle* spawn() {
        // 线性探测下一个空闲槽；循环队列式的 next 指针摊平查找成本。
        for (std::size_t i = 0; i < slots_.size(); ++i) {
            Particle& slot = slots_[(next_ + i) % slots_.size()];
            if (!slot.active) {
                slot.active = true;
                next_ = (next_ + i + 1) % slots_.size();
                ++spawn_count_;
                return &slot;
            }
        }
        return nullptr; // 池满：按策略放弃或复用最旧粒子
    }

    void kill(Particle* particle) { particle->active = false; }

    std::size_t active_count() const {
        std::size_t count = 0;
        for (const Particle& p : slots_) {
            count += p.active ? 1 : 0;
        }
        return count;
    }
    std::size_t capacity() const { return slots_.size(); }
    std::size_t spawn_count() const { return spawn_count_; }

private:
    std::vector<Particle> slots_;
    std::size_t next_ = 0;
    std::size_t spawn_count_ = 0;
};

void object_pool_demo() {
    ParticlePool pool(4);

    Particle* first = pool.spawn();
    Particle* second = pool.spawn();
    Particle* third = pool.spawn();
    Particle* fourth = pool.spawn();
    REQUIRE(pool.active_count() == 4);
    REQUIRE(pool.spawn() == nullptr); // 池满拒绝：绝不临时扩容

    pool.kill(second);
    Particle* reused = pool.spawn();
    REQUIRE(reused == second); // 复用刚释放的槽位，地址与分配次数都不变
    REQUIRE(pool.spawn_count() == 5);
    (void)first;
    (void)third;
    (void)fourth;
}

// ---------------------------------------------------------------------------
// 4. Spatial Partition：均匀网格把近邻查询从 O(n²) 降到近似 O(n)
// ---------------------------------------------------------------------------

struct Point {
    double x = 0.0;
    double y = 0.0;
};

std::size_t count_pairs_bruteforce(const std::vector<Point>& points,
                                   double radius) {
    std::size_t pairs = 0;
    for (std::size_t i = 0; i < points.size(); ++i) {
        for (std::size_t j = i + 1; j < points.size(); ++j) {
            const double dx = points[i].x - points[j].x;
            const double dy = points[i].y - points[j].y;
            if (std::sqrt(dx * dx + dy * dy) <= radius) {
                ++pairs;
            }
        }
    }
    return pairs;
}

class UniformGrid {
public:
    UniformGrid(const std::vector<Point>& points, double world_size,
                double cell_size)
        : cell_size_(cell_size),
          cells_per_side_(static_cast<int>(world_size / cell_size) + 1),
          cells_(static_cast<std::size_t>(cells_per_side_ * cells_per_side_)) {
        for (std::size_t i = 0; i < points.size(); ++i) {
            cells_[cell_index(points[i])].push_back(i);
        }
    }

    // 只检查同格与相邻格：候选数从 n 个降到平均每格几个。
    std::size_t count_pairs(const std::vector<Point>& points,
                            double radius) const {
        std::size_t pairs = 0;
        for (const auto& cell : cells_) {
            for (std::size_t a = 0; a < cell.size(); ++a) {
                for (std::size_t b = a + 1; b < cell.size(); ++b) {
                    pairs += close(points[cell[a]], points[cell[b]], radius);
                }
            }
        }
        // 相邻格两两去重：检查 8 个邻居中的 4 个方向（右、下、右下、
        // 左下）即可覆盖全部跨格配对，不重复计数。对角格也必须查：
        // 对角两点的距离可以小于半径。
        constexpr int kOffsets[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
        for (int cy = 0; cy < cells_per_side_; ++cy) {
            for (int cx = 0; cx < cells_per_side_; ++cx) {
                const auto& here = cells_[cell_index(cx, cy)];
                for (const auto& off : kOffsets) {
                    const int nx = cx + off[0];
                    const int ny = cy + off[1];
                    if (nx < 0 || nx >= cells_per_side_ || ny < 0 ||
                        ny >= cells_per_side_) {
                        continue;
                    }
                    for (const std::size_t i : here) {
                        for (const std::size_t j :
                             cells_[cell_index(nx, ny)]) {
                            pairs += close(points[i], points[j], radius);
                        }
                    }
                }
            }
        }
        return pairs;
    }

private:
    static bool close(const Point& a, const Point& b, double radius) {
        const double dx = a.x - b.x;
        const double dy = a.y - b.y;
        return dx * dx + dy * dy <= radius * radius;
    }
    int cell_of(double v) const {
        return std::max(0, static_cast<int>(v / cell_size_));
    }
    std::size_t cell_index(const Point& p) const {
        return cell_index(cell_of(p.x), cell_of(p.y));
    }
    std::size_t cell_index(int cx, int cy) const {
        return static_cast<std::size_t>(cy * cells_per_side_ + cx);
    }

    double cell_size_;
    int cells_per_side_;
    std::vector<std::vector<std::size_t>> cells_;
};

void spatial_partition_demo() {
    // 固定种子：结果可复现，CTest 里可以精确断言。
    std::mt19937 rng{20261118u};
    std::uniform_real_distribution<double> dist(0.0, 100.0);

    constexpr std::size_t kPoints = 1000;
    std::vector<Point> points;
    points.reserve(kPoints);
    for (std::size_t i = 0; i < kPoints; ++i) {
        points.push_back(Point{dist(rng), dist(rng)});
    }

    const double radius = 2.0;
    const auto start = std::chrono::steady_clock::now();
    const std::size_t brute = count_pairs_bruteforce(points, radius);
    const auto mid = std::chrono::steady_clock::now();
    UniformGrid grid(points, 100.0, radius); // 格边长取查询半径，一查 9 格
    const std::size_t gridded = grid.count_pairs(points, radius);
    const auto end = std::chrono::steady_clock::now();

    REQUIRE(brute == gridded); // 结果必须一致，网格只改查询方式不改语义

    auto micros = [](auto d) {
        return std::chrono::duration_cast<std::chrono::microseconds>(d)
            .count();
    };
    std::cout << "pairs=" << brute << " brute=" << micros(mid - start)
              << " us, grid=" << micros(end - mid) << " us\n";
}

int main() {
    update_method_demo();
    observer_and_event_queue_demo();
    object_pool_demo();
    spatial_partition_demo();
    std::cout << "ch04_game_patterns: all checks passed\n";
    return 0;
}
