#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <iostream>
#include <mutex>
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
// 教学版任务系统：依赖 DAG + fork/join + work stealing
//
// 生产实现（UE Tasks System、Unity Job System、Chase-Lev 队列）用无锁
// 队列与自动批量偷取；本实现用「每 worker 双端队列 + 互斥锁」近似同
// 样的调度结构，把重点放在依赖、join 纪律与偷取方向上。
// ---------------------------------------------------------------------------

class JobSystem {
public:
    struct Job {
        std::function<void()> fn;
        std::atomic<std::uint32_t> pending{0}; // 未满足的依赖数
        std::atomic<bool> completed{false};
        std::vector<Job*> dependents; // 完成后要通知的后继任务
    };

    explicit JobSystem(std::size_t worker_count)
        : queues_(worker_count), steal_count_(worker_count, 0) {
        for (std::size_t i = 0; i < worker_count; ++i) {
            workers_.emplace_back([this, i] { worker_loop(i); });
        }
    }

    ~JobSystem() {
        {
            std::lock_guard<std::mutex> lock(wakeup_mutex_);
            stop_ = true;
        }
        wakeup_cv_.notify_all();
        for (std::thread& worker : workers_) {
            worker.join();
        }
    }

    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;

    // 提交任务：fn 执行体，deps 依赖集合（依赖全部完成后才可调度）。
    // 可以在依赖完成之后提交后继任务——注册时会检查 completed。
    Job* submit(std::function<void()> fn, const std::vector<Job*>& deps = {}) {
        Job* job = allocate(std::move(fn));
        for (Job* dep : deps) {
            std::lock_guard<std::mutex> lock(deps_mutex_);
            if (!dep->completed.load(std::memory_order_acquire)) {
                dep->dependents.push_back(job);
                job->pending.fetch_add(1, std::memory_order_relaxed);
            }
        }
        if (job->pending.load(std::memory_order_relaxed) == 0) {
            enqueue(job);
        }
        return job;
    }

    void wait(Job* job) {
        // 教学版自旋等待；生产实现会挂起线程或用任务依赖接管当前任务，
        // 避免空转烧 CPU——这也是「优先建图、少显式等待」的原因之一。
        while (!job->completed.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
    }

    // fork/join 主形态：把范围切成块并行执行，fan-in 一个空任务做 join。
    template <typename Fn>
    void parallel_for(int begin, int end, int chunk_size, Fn&& fn) {
        std::vector<Job*> chunks;
        for (int b = begin; b < end; b += chunk_size) {
            const int e = std::min(b + chunk_size, end);
            chunks.push_back(submit([&fn, b, e] {
                for (int i = b; i < e; ++i) {
                    fn(i);
                }
            }));
        }
        wait(submit([] {}, chunks)); // join 点：到这里才能读共享结果
    }

    std::size_t steal_count(std::size_t worker) const {
        return steal_count_[worker];
    }

private:
    Job* allocate(std::function<void()> fn) {
        std::lock_guard<std::mutex> lock(alloc_mutex_);
        jobs_.emplace_back();
        jobs_.back().fn = std::move(fn);
        return &jobs_.back();
    }

    void enqueue(Job* job) {
        // 主线程/worker 统一走轮转派发；worker 闲置时会偷别人的队尾。
        const std::size_t index =
            next_queue_.fetch_add(1, std::memory_order_relaxed) %
            queues_.size();
        {
            std::lock_guard<std::mutex> lock(queues_[index].mutex);
            queues_[index].tasks.push_back(job);
        }
        {
            std::lock_guard<std::mutex> lock(wakeup_mutex_);
            work_hint_ = true;
        }
        wakeup_cv_.notify_all();
    }

    Job* try_pop_local(std::size_t worker) {
        std::lock_guard<std::mutex> lock(queues_[worker].mutex);
        if (queues_[worker].tasks.empty()) {
            return nullptr;
        }
        Job* job = queues_[worker].tasks.front();
        queues_[worker].tasks.pop_front();
        return job; // 本端从队首取：最后提交的最先执行（LIFO，缓存热）
    }

    Job* try_steal(std::size_t worker) {
        const std::size_t count = queues_.size();
        for (std::size_t offset = 1; offset < count; ++offset) {
            const std::size_t victim = (worker + offset) % count;
            std::lock_guard<std::mutex> lock(queues_[victim].mutex);
            if (queues_[victim].tasks.empty()) {
                continue;
            }
            Job* job = queues_[victim].tasks.back();
            queues_[victim].tasks.pop_back();
            ++steal_count_[worker]; // 他端从队尾偷（FIFO，公平）
            return job;
        }
        return nullptr;
    }

    void worker_loop(std::size_t worker) {
        for (;;) {
            Job* job = try_pop_local(worker);
            if (job == nullptr) {
                job = try_steal(worker);
            }
            if (job == nullptr) {
                bool should_stop = false;
                {
                    std::unique_lock<std::mutex> lock(wakeup_mutex_);
                    wakeup_cv_.wait_for(lock, std::chrono::milliseconds(1),
                                        [this] { return stop_ || work_hint_; });
                    should_stop = stop_;
                    work_hint_ = false;
                }
                if (should_stop && try_pop_local(worker) == nullptr &&
                    try_steal(worker) == nullptr) {
                    return; // 收到停止且再无任务
                }
                continue;
            }
            run(*job);
        }
    }

    void run(Job& job) {
        job.fn();
        std::vector<Job*> to_wake;
        {
            std::lock_guard<std::mutex> lock(deps_mutex_);
            job.completed.store(true, std::memory_order_release);
            to_wake = job.dependents;
        }
        for (Job* dependent : to_wake) {
            // 从 1 减到 0 的那次减法负责入队：依赖恰好满足一次。
            if (dependent->pending.fetch_sub(1, std::memory_order_acq_rel) ==
                1) {
                enqueue(dependent);
            }
        }
    }

    struct WorkerQueue {
        std::mutex mutex;
        std::deque<Job*> tasks;
    };

    std::vector<std::thread> workers_;
    std::vector<WorkerQueue> queues_;
    std::vector<std::size_t> steal_count_;
    std::deque<Job> jobs_; // 追加式存储：Job 指针在提交后永远稳定
    std::mutex alloc_mutex_;
    std::mutex deps_mutex_;
    std::atomic<std::size_t> next_queue_{0};
    std::mutex wakeup_mutex_;
    std::condition_variable wakeup_cv_;
    bool work_hint_ = false;
    bool stop_ = false;
};

// ---------------------------------------------------------------------------
// 1. 依赖 DAG：A → B → C，执行顺序被依赖强制
// ---------------------------------------------------------------------------

void dag_order_demo() {
    JobSystem system{4};
    std::atomic<int> tick{0};
    std::mutex record_mutex;
    int order_of[3] = {0, 0, 0};

    JobSystem::Job* a = system.submit([&] {
        std::lock_guard<std::mutex> lock(record_mutex);
        order_of[0] = tick.fetch_add(1);
    });
    JobSystem::Job* b = system.submit(
        [&] {
            std::lock_guard<std::mutex> lock(record_mutex);
            order_of[1] = tick.fetch_add(1);
        },
        {a});
    JobSystem::Job* c = system.submit(
        [&] {
            std::lock_guard<std::mutex> lock(record_mutex);
            order_of[2] = tick.fetch_add(1);
        },
        {a, b});

    system.wait(c); // join 点：C 完成蕴含 A、B 全部完成

    REQUIRE(order_of[0] < order_of[1]);
    REQUIRE(order_of[1] < order_of[2]);
}

// ---------------------------------------------------------------------------
// 2. fork/join：parallel_for 拆分 + 帧末统一读结果
// ---------------------------------------------------------------------------

void parallel_for_demo() {
    JobSystem system{4};
    constexpr int kCount = 1000;
    std::vector<int> squares(static_cast<std::size_t>(kCount), 0);

    // 动画/物理式「写分离」：每块只写自己的下标区间，无需加锁。
    system.parallel_for(0, kCount, 128, [&squares](int i) {
        squares[static_cast<std::size_t>(i)] = i * i;
    });

    // join 已在 parallel_for 内部完成：现在读结果才合法。
    for (int i = 0; i < kCount; ++i) {
        REQUIRE(squares[static_cast<std::size_t>(i)] == i * i);
    }
}

// ---------------------------------------------------------------------------
// 3. work stealing：轮转派发 + 闲置 worker 偷队尾
// ---------------------------------------------------------------------------

void work_stealing_demo() {
    JobSystem system{4};
    constexpr int kJobs = 2000;
    std::atomic<int> executed{0};

    std::vector<JobSystem::Job*> jobs;
    jobs.reserve(static_cast<std::size_t>(kJobs));
    for (int i = 0; i < kJobs; ++i) {
        jobs.push_back(system.submit([&executed] { executed.fetch_add(1); }));
    }
    for (JobSystem::Job* job : jobs) {
        system.wait(job);
    }

    REQUIRE(executed.load() == kJobs); // 每个任务恰好执行一次
    std::size_t total_steals = 0;
    for (std::size_t i = 0; i < 4; ++i) {
        total_steals += system.steal_count(i);
    }
    std::cout << "work stealing: " << total_steals << " steals over "
              << kJobs << " jobs\n"; // 只观察，不断言具体次数
}

// ---------------------------------------------------------------------------
// 4. 帧同步点纪律：任务全部 join 之后，游戏线程才读权威状态
// ---------------------------------------------------------------------------

void frame_sync_demo() {
    JobSystem system{4};
    constexpr int kEntities = 512;
    std::vector<double> positions(static_cast<std::size_t>(kEntities), 0.0);

    // 帧内并行：动画与「物理」各管一段下标，互不重叠。
    system.parallel_for(0, kEntities, 64, [&positions](int i) {
        positions[static_cast<std::size_t>(i)] += 1.0; // 动画推进
    });

    // 提交渲染相关任务前，先确认模拟状态可见：join 已保证。
    double checksum = 0.0;
    for (double p : positions) {
        checksum += p;
    }
    REQUIRE(checksum == static_cast<double>(kEntities));

    // 反例（不要这样做）：submit 一个写 positions 的任务后立即读，
    // 不 join 就读到旧值或竞态。join 点是读共享结果的唯一合法时机。
}

int main() {
    dag_order_demo();
    parallel_for_demo();
    work_stealing_demo();
    frame_sync_demo();
    std::cout << "ch04_job_system: all checks passed\n";
    return 0;
}
