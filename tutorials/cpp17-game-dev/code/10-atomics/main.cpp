#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>
#include <type_traits>
#include <vector>

#define REQUIRE(cond)                                                          \
    do {                                                                       \
        if (!(cond)) {                                                         \
            std::cerr << "REQUIRE failed: " #cond " at line " << __LINE__      \
                      << '\n';                                                 \
            std::abort();                                                      \
        }                                                                      \
    } while (0)

// 多数平台 destructively interference size 为 64；标准常量在部分实现上
// 仍不可移植，教程用固定值并注明。
constexpr std::size_t kCacheLine = 64;

// ---------------------------------------------------------------------------
// 1. 原子操作基础：load / store / fetch_add / CAS 循环
// ---------------------------------------------------------------------------

void atomic_basics() {
    std::atomic<int> counter{0};

    counter.store(10, std::memory_order_relaxed);
    counter.fetch_add(5, std::memory_order_relaxed);
    REQUIRE(counter.load(std::memory_order_relaxed) == 15);

    // CAS 循环：与别的线程竞争写权，输掉就拿到最新值重试。
    int expected = 15;
    const bool won = counter.compare_exchange_strong(expected, 20);
    REQUIRE(won && expected == 15);
    REQUIRE(counter.load() == 20);

    const bool lost = counter.compare_exchange_strong(expected, 99);
    REQUIRE(!lost); // expected 被更新为当前值
    REQUIRE(expected == 20);
    REQUIRE(counter.load() == 20);

    // atomic<T> 不可复制不可移动，防止「复制的瞬间值已过期」。
    static_assert(!std::is_copy_constructible<std::atomic<int>>::value,
                  "atomic 不可复制");
}

// ---------------------------------------------------------------------------
// 2. relaxed 计数：自身原子，但不提供跨线程同步
// ---------------------------------------------------------------------------

void relaxed_counter() {
    constexpr int kThreads = 4;
    constexpr int kIncrements = 10000;

    std::atomic<int> hits{0};
    std::atomic<bool> done{false};
    {
        std::vector<std::thread> workers;
        for (int t = 0; t < kThreads; ++t) {
            workers.emplace_back([&hits] {
                for (int i = 0; i < kIncrements; ++i) {
                    hits.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }
        for (auto& worker : workers) {
            worker.join();
        }
        done.store(true, std::memory_order_release);
    }
    // join 自带同步；release store + acquire load 保证读方看到全部计数。
    REQUIRE(done.load(std::memory_order_acquire));
    REQUIRE(hits.load(std::memory_order_relaxed) == kThreads * kIncrements);

    // relaxed 的正确场合：只关心最终总和、不借它传递其他数据的计数。
    // 错误场合：用 relaxed 写标志、却期望另一线程看到标志前的普通写入。
}

// ---------------------------------------------------------------------------
// 3. acquire/release：跨线程发布数据的标准配对
// ---------------------------------------------------------------------------

struct FramePacket {
    int entity_count = 0;
    double avg_frame_time = 0.0;
};

void release_acquire_publish() {
    // 游戏场景：工作线程「打包」数据，游戏线程在发布后读取。
    FramePacket packet; // 普通变量：访问受下面的 release/acquire 保护
    std::atomic<FramePacket*> published{nullptr};

    std::thread producer([&packet, &published] {
        packet.entity_count = 512; // 1) 先写普通数据
        packet.avg_frame_time = 16.6;
        published.store(&packet, std::memory_order_release); // 2) 再发布
    });

    std::thread consumer([&published] {
        FramePacket* seen = published.load(std::memory_order_acquire); // 3) 后读取
        if (seen != nullptr) {
            // acquire 保证：release 之前的所有写（含普通写）对本线程可见。
            REQUIRE(seen->entity_count == 512);
            REQUIRE(seen->avg_frame_time == 16.6);
        }
    });

    producer.join();
    consumer.join();
    REQUIRE(published.load() == &packet);

    // 如果 consumer 在发布前就 load 到 nullptr，说明它先于生产者运行；
    // 本例用 main 的第二次读取兜底断言最终可见。
    FramePacket* seen = published.load(std::memory_order_acquire);
    REQUIRE(seen == &packet);
    REQUIRE(seen->entity_count == 512);
}

// ---------------------------------------------------------------------------
// 4. atomic_flag 自旋锁：最轻量的锁原语
// ---------------------------------------------------------------------------

void spinlock_demo() {
    std::atomic_flag flag = ATOMIC_FLAG_INIT; // C++17 必须显式初始化为 false
    int shared_total = 0;

    auto lock = [&flag] {
        while (flag.test_and_set(std::memory_order_acquire)) {
            // 自旋：忙等直到 test_and_set 返回 false
        }
    };
    auto unlock = [&flag] { flag.clear(std::memory_order_release); };

    std::vector<std::thread> workers;
    for (int t = 0; t < 4; ++t) {
        workers.emplace_back([&] {
            for (int i = 0; i < 1000; ++i) {
                lock();
                ++shared_total;
                unlock();
            }
        });
    }
    for (auto& worker : workers) {
        worker.join();
    }
    REQUIRE(shared_total == 4000);

    // 自旋锁适合「临界区极短、竞争者少」的场景；长临界区空转会烧 CPU，
    // 应改用 mutex（内核可调度等待者）。
}

// ---------------------------------------------------------------------------
// 5. false sharing：不同线程写同一 cache line 的不同变量
// ---------------------------------------------------------------------------

#if defined(_MSC_VER)
// alignas 导致的填充是本示例的刻意设计，不是意外膨胀。
#pragma warning(disable : 4324)
#endif

struct PackedCounters {
    std::atomic<long long> a{0};
    std::atomic<long long> b{0};
};

struct alignas(kCacheLine) Padded {
    std::atomic<long long> value{0};
};
static_assert(sizeof(Padded) == kCacheLine, "Padded 独占一条 cache line");

template <typename Counters>
long long run_counters(Counters& counters, int rounds) {
    auto start = std::chrono::steady_clock::now();
    std::vector<std::thread> workers;
    workers.emplace_back([&counters, rounds] {
        for (int i = 0; i < rounds; ++i) {
            counters.a.fetch_add(1, std::memory_order_relaxed);
        }
    });
    workers.emplace_back([&counters, rounds] {
        for (int i = 0; i < rounds; ++i) {
            counters.b.fetch_add(1, std::memory_order_relaxed);
        }
    });
    for (auto& worker : workers) {
        worker.join();
    }
    auto elapsed = std::chrono::steady_clock::now() - start;
    return std::chrono::duration_cast<std::chrono::microseconds>(elapsed)
        .count();
}

void false_sharing_measurement() {
    constexpr int kRounds = 2000000;

    PackedCounters packed;
    const long long packed_us = run_counters(packed, kRounds);
    REQUIRE(packed.a.load() == kRounds);
    REQUIRE(packed.b.load() == kRounds);

    Padded pad_a;
    Padded pad_b;
    struct Separated {
        std::atomic<long long>& a;
        std::atomic<long long>& b;
    } separated{pad_a.value, pad_b.value};
    const long long padded_us = run_counters(separated, kRounds);
    REQUIRE(pad_a.value.load() == kRounds);
    REQUIRE(pad_b.value.load() == kRounds);

    // a 与 b 相距 64 字节以内时落在同一条 cache line：两个核来回抢夺
    // 所有权（cache line ping-pong），吞吐显著下降。填充到不同 line 后
    // 各写各的。只打印测量值不做性能断言：比例随机器而异。
    std::cout << "packed: " << packed_us << " us, padded: " << padded_us
              << " us\n";
}

int main() {
    atomic_basics();
    relaxed_counter();
    release_acquire_publish();
    spinlock_demo();
    false_sharing_measurement();
    std::cout << "ch03_atomics: all checks passed\n";
    return 0;
}
