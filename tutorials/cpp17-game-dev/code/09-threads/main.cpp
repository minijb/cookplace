#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
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
// 1. std::thread 生命周期：必须 join 或 detach，否则 terminate
// ---------------------------------------------------------------------------

// RAII 守护：作用域结束时自动 join，杜绝忘 join。
// 变参构造：直接接受可调用体，内部代为构造 std::thread（thread 的
// 构造函数是 explicit，不能隐式两步转换）。
class JoinGuard {
public:
    template <typename F, typename... Args>
    explicit JoinGuard(F&& entry, Args&&... args)
        : thread_(std::forward<F>(entry), std::forward<Args>(args)...) {}
    ~JoinGuard() {
        if (thread_.joinable()) {
            thread_.join();
        }
    }
    JoinGuard(const JoinGuard&) = delete;
    JoinGuard& operator=(const JoinGuard&) = delete;
    JoinGuard(JoinGuard&& other) noexcept : thread_(std::move(other.thread_)) {}
    std::thread& get() { return thread_; }

private:
    std::thread thread_;
};

void thread_basics() {
    int result = 0;
    std::mutex result_mutex;

    // 把结果写回调用方状态时必须同步；按引用捕获 &result、&result_mutex。
    JoinGuard worker([&result, &result_mutex] {
        std::lock_guard<std::mutex> lock(result_mutex);
        result = 40 + 2;
    });
    REQUIRE(worker.get().joinable()); // 未 join 前是 joinable 的

    worker.get().join();
    REQUIRE(!worker.get().joinable()); // join 之后不再 joinable
    REQUIRE(result == 42);

    // 教程示例一律 join，绝不 detach：detach 的线程与对象生命周期脱钩，
    // 引用局部变量就是悬垂访问。
}

// ---------------------------------------------------------------------------
// 2. 数据竞争：无同步的共享写是 UB，用 mutex 或 atomic 修复
// ---------------------------------------------------------------------------

void race_and_fix() {
    constexpr int kThreads = 4;
    constexpr int kIncrements = 10000;

    // 修复方案 A：互斥量串行化临界区。
    int guarded = 0;
    std::mutex mutex;
    {
        std::vector<JoinGuard> workers;
        for (int t = 0; t < kThreads; ++t) {
            workers.emplace_back([&guarded, &mutex] {
                for (int i = 0; i < kIncrements; ++i) {
                    std::lock_guard<std::mutex> lock(mutex);
                    ++guarded;
                }
            });
        }
    } // JoinGuard 析构逐个 join
    REQUIRE(guarded == kThreads * kIncrements);

    // 修复方案 B：原子变量。fetch_add 是原子的读-改-写。
    std::atomic<int> counter{0};
    {
        std::vector<JoinGuard> workers;
        for (int t = 0; t < kThreads; ++t) {
            workers.emplace_back([&counter] {
                for (int i = 0; i < kIncrements; ++i) {
                    counter.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }
    }
    REQUIRE(counter.load() == kThreads * kIncrements);

    // 注意：counter++ 也不行——它是一次 load 加一次 store，两步之间
    // 别的线程可以插进来；必须用 fetch_add 或 CAS 循环。
    // 无同步版本的现场演示见 ch03_race_demo（配合 TSan 运行），故意不进
    // 自动测试：数据竞争是 UB，结果不可断言。
}

// ---------------------------------------------------------------------------
// 3. 死锁：以固定顺序加锁，或用 std::scoped_lock 一次锁多个
// ---------------------------------------------------------------------------

void deadlock_avoidance() {
    std::mutex inventory_mutex;
    std::mutex quest_mutex;
    int gold = 100;
    int quest_progress = 3;

    // 两个线程分别按「inventory→quest」和「quest→inventory」加锁就可能
    // 死锁。C++17 的 scoped_lock 用内部算法同时获取多个锁，避免环路等待。
    {
        std::vector<JoinGuard> workers;
        workers.emplace_back([&] {
            std::scoped_lock lock(inventory_mutex, quest_mutex);
            gold += 50;
            quest_progress += 1;
        });
        workers.emplace_back([&] {
            std::scoped_lock lock(quest_mutex, inventory_mutex); // 反向也不死锁
            quest_progress = quest_progress > 0 ? quest_progress - 1 : 0;
            gold -= 10;
        });
    }
    REQUIRE(gold == 140);
    REQUIRE(quest_progress == 3);
}

// ---------------------------------------------------------------------------
// 4. condition_variable：生产者-消费者与虚假唤醒
// ---------------------------------------------------------------------------

void producer_consumer() {
    constexpr int kItems = 100;
    std::queue<int> tasks;
    std::mutex mutex;
    std::condition_variable ready;
    bool producer_done = false;
    std::vector<int> consumed;

    // 谓词版 wait 等价于「循环 + 虚假唤醒防护」：
    //   while (!pred) cv.wait(lock);
    auto consumer = [&] {
        std::unique_lock<std::mutex> lock(mutex);
        for (;;) {
            ready.wait(lock, [&] { return !tasks.empty() || producer_done; });
            while (!tasks.empty()) {
                consumed.push_back(tasks.front());
                tasks.pop();
            }
            if (producer_done && tasks.empty()) {
                return;
            }
        }
    };

    JoinGuard worker(consumer);
    {
        std::lock_guard<std::mutex> lock(mutex);
        for (int i = 0; i < kItems; ++i) {
            tasks.push(i);
        }
        producer_done = true;
    }
    ready.notify_one();

    worker.get().join();
    REQUIRE(consumed.size() == static_cast<std::size_t>(kItems));
    for (int i = 0; i < kItems; ++i) {
        REQUIRE(consumed[static_cast<std::size_t>(i)] == i);
    }
}

int main(int argc, char** argv) {
    thread_basics();
    race_and_fix();
    deadlock_avoidance();
    producer_consumer();

    // 仅在显式传入 race 时运行无保护计数演示（供 TSan 观察用）。
    if (argc > 1 && std::string{argv[1]} == "race") {
        int unsafe = 0;
        std::vector<std::thread> workers;
        for (int t = 0; t < 4; ++t) {
            workers.emplace_back([&unsafe] {
                for (int i = 0; i < 10000; ++i) {
                    ++unsafe; // 数据竞争：结果通常小于 40000
                }
            });
        }
        for (auto& t : workers) {
            t.join();
        }
        std::cout << "unsafe counter (expected 40000): " << unsafe << '\n';
    }

    std::cout << "ch03_threads: all checks passed\n";
    return 0;
}
