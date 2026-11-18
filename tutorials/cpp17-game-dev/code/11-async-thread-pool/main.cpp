#include <chrono>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#define REQUIRE(cond)                                                          \
    do {                                                                       \
        if (!(cond)) {                                                         \
            std::cerr << "REQUIRE failed: " #cond " at line " << __LINE__      \
                      << '\n';                                                 \
            std::abort();                                                      \
        }                                                                      \
    } while (0)

using Clock = std::chrono::steady_clock;

// ---------------------------------------------------------------------------
// 1. std::async 的 launch policy：async 开新线程，deferred 惰性执行
// ---------------------------------------------------------------------------

void launch_policies() {
    const std::thread::id caller_id = std::this_thread::get_id();

    // launch::async：保证在新线程上运行。
    std::future<std::thread::id> async_id =
        std::async(std::launch::async, [] { return std::this_thread::get_id(); });
    REQUIRE(async_id.get() != caller_id);

    // launch::deferred：不执行，直到 get/wait 时在「调用线程」上执行。
    std::future<std::thread::id> deferred_id =
        std::async(std::launch::deferred,
                   [] { return std::this_thread::get_id(); });
    REQUIRE(deferred_id.wait_for(std::chrono::seconds(0)) ==
            std::future_status::deferred); // 尚未启动
    REQUIRE(deferred_id.get() == caller_id);

    // 不传 policy（默认 async|deferred）：实现裁量是否并发，行为不可控；
    // 需要并发就显式写 std::launch::async。
}

// ---------------------------------------------------------------------------
// 2. 高频坑：std::async 返回的 future 析构会阻塞等待任务完成
// ---------------------------------------------------------------------------

void discard_future_blocks() {
    constexpr auto kTaskDuration = std::chrono::milliseconds(100);

    auto slow_task = [kTaskDuration] {
        std::this_thread::sleep_for(kTaskDuration);
        return 1;
    };

    // 「fire and forget」写法是假的：丢弃的 async future 在析构时逐个
    // 阻塞到任务结束，整体退化为串行执行。MSVC（/W4）甚至会对直接丢弃
    // 返回值发 C4858 警告；这里用即逝变量保持同一语义，保持跨编译器安
    // 静。直接写 std::async(...) 不接返回值，效果相同且更能触发编译器
    // 提醒，值得记住。
    const auto start = Clock::now();
    for (int i = 0; i < 2; ++i) {
        std::future<int> discarded = std::async(std::launch::async, slow_task);
        (void)discarded; // 立即离开作用域：析构阻塞等待任务完成
    }
    const auto elapsed = Clock::now() - start;
    REQUIRE(elapsed >= kTaskDuration + kTaskDuration / 2); // 宽松下限

    // deferred future 被丢弃则任务根本不执行（惰性任务直接作废）。
    {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4702) // abort 之后的 return 0 不可达是刻意的
#endif
        std::future<int> never = std::async(std::launch::deferred, [] {
            std::abort(); // 若执行到这里即失败
            return 0;
        });
        (void)never; // 丢弃：不执行、不阻塞
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    }

    std::cout << "discard loop took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed)
                     .count()
              << " ms (blocked on two "
              << kTaskDuration.count() << " ms tasks)\n";
}

// ---------------------------------------------------------------------------
// 3. promise/future 与异常传递
// ---------------------------------------------------------------------------

void promise_and_exceptions() {
    std::promise<int> promise;
    std::future<int> future = promise.get_future();

    std::thread producer([&promise] {
        try {
            throw std::runtime_error("asset decode failed");
        } catch (...) {
            promise.set_exception(std::current_exception()); // 异常存入共享状态
        }
    });
    producer.join();

    try {
        (void)future.get(); // get() 在消费线程重抛
        REQUIRE(false);     // 不应到达
    } catch (const std::runtime_error& e) {
        REQUIRE(std::string{e.what()} == "asset decode failed");
    }
    REQUIRE(!future.valid()); // get 之后 future 失效；多处消费用 shared_future

    // 不 get 异常就永久吞掉——异步路径必须显式消费结果。
}

// ---------------------------------------------------------------------------
// 4. packaged_task：把「可调用体 + future」打包成手动执行的任务
// ---------------------------------------------------------------------------

void packaged_task_basics() {
    std::packaged_task<int(int, int)> task{
        [](int a, int b) { return a + b; }};
    std::future<int> result = task.get_future();

    task(20, 22); // 手动执行（这里在当前线程）；执行完结果自动进入 future
    REQUIRE(result.get() == 42);
}

// ---------------------------------------------------------------------------
// 5. 手写线程池：mutex + condition_variable + packaged_task
// ---------------------------------------------------------------------------

class ThreadPool {
public:
    explicit ThreadPool(std::size_t thread_count) {
        if (thread_count == 0) {
            thread_count = 1;
        }
        workers_.reserve(thread_count);
        for (std::size_t i = 0; i < thread_count; ++i) {
            workers_.emplace_back([this] { worker_loop(); });
        }
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_ = true; // 先等队列排空再退出（worker_loop 的退出条件）
        }
        cv_.notify_all();
        for (std::thread& worker : workers_) {
            worker.join();
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    template <typename F>
    auto submit(F&& task) -> std::future<std::invoke_result_t<std::decay_t<F>&>> {
        using Result = std::invoke_result_t<std::decay_t<F>&>;
        std::packaged_task<Result()> packaged{std::forward<F>(task)};
        std::future<Result> future = packaged.get_future();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stop_) {
                throw std::runtime_error("submit on stopped thread pool");
            }
            // packaged_task 可搬移不可复制；用它把任意可调用体擦除成统一形态。
            tasks_.emplace([task = std::move(packaged)]() mutable { task(); });
        }
        cv_.notify_one();
        return future;
    }

private:
    void worker_loop() {
        for (;;) {
            std::packaged_task<void()> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                // 谓词版 wait：防虚假唤醒，退出条件是「stop 且队列已空」。
                cv_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                if (stop_ && tasks_.empty()) {
                    return;
                }
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            task(); // 在锁外执行，避免长任务阻塞队列
        }
    }

    std::vector<std::thread> workers_;
    std::queue<std::packaged_task<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_ = false;
};

// ---------------------------------------------------------------------------
// 6. 游戏场景：帧末 join 的异步加载
// ---------------------------------------------------------------------------

void frame_end_join() {
    ThreadPool pool{std::max(2u, std::thread::hardware_concurrency())};

    // 提交 8 个假想资源加载：不阻塞游戏线程，帧末统一收结果。
    std::vector<std::future<int>> loads;
    loads.reserve(8);
    for (int id = 0; id < 8; ++id) {
        loads.push_back(pool.submit([id] {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            return id * 2; // 「解码结果」
        }));
    }

    // 帧末同步点：到这里才读结果；future::get 各自阻塞到任务完成。
    for (int id = 0; id < 8; ++id) {
        REQUIRE(loads[static_cast<std::size_t>(id)].get() == id * 2);
    }

    std::cout << "frame_end_join: 8 loads joined\n";
}

int main() {
    launch_policies();
    discard_future_blocks();
    promise_and_exceptions();
    packaged_task_basics();
    frame_end_join();
    std::cout << "ch03_async_thread_pool: all checks passed\n";
    return 0;
}
