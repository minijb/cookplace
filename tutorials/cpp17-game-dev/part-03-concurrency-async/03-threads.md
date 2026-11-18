---
title: 第三章（中下）：线程与数据竞争
series: cpp17-game-dev
chapter: 第三章（中下）
topic: std::thread、mutex、condition_variable 与数据竞争的现场观察
audience: 已掌握模板基础、要为游戏逻辑引入并发的前端游戏开发工程师
outcome: 能安全地启动与收束线程，用互斥量保护共享状态，用条件变量实现等待，并用 TSan 定位数据竞争
environment: C++17；CMake 3.16+；Windows + MSVC 19.50 已验证；2026-11-18
date: 2026-11-18
---
# 第三章（中下）：线程与数据竞争

适用环境：C++17、CMake 3.16 或更高版本；示例已在 Windows + MSVC 19.50 下验证。

你将学会：

- 用 RAII 守护线程生命周期，杜绝忘 `join`。
- 解释数据竞争为什么是未定义行为，并亲手用 TSan 观察一份报告。
- 在互斥量、原子变量之间为简单场景做出正确选择。
- 用条件变量写生产者-消费者，并防住虚假唤醒。
- 用固定加锁顺序或 `std::scoped_lock` 避免死锁。

完整代码位于 [线程示例](../code/09-threads/main.cpp)；数据竞争演示程序见 [race_demo.cpp](../code/09-threads/race_demo.cpp)。

## 1. 线程的生命周期：必须 join 或 detach

`std::thread` 对象析构时若线程仍 `joinable()`，程序直接 `std::terminate`。规则只有两条：要么 `join()`（等待结束），要么 `detach()`（放养，与对象脱钩）。教程示例一律 `join`——`detach` 的线程引用局部变量就是悬垂访问，几乎没有安全用法。

用 RAII 把 join 绑进作用域，比手工记得 join 可靠得多：

```cpp
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
private:
    std::thread thread_;
};
```

这与第一章 RAII 的思路完全一致：资源（这里是「一个尚未收束的执行流」）的生命周期绑进对象。注意 `std::thread` 的构造函数是 `explicit` 的，所以守护类用变参构造代为创建线程，不能依赖隐式两步转换。

> ⚠️ `join()` 只能调用一次；重复调用或对已 join 的线程调用都会抛 `std::invalid_argument`。判断依据是 `joinable()`，不是「我觉得它结束了」。

## 2. 数据竞争：最小的错误，最深的坑

两个线程并发访问同一内存位置、至少一个是写、且没有任何同步——这就是数据竞争，标准把它定义为**未定义行为**。不是「结果随机」，而是整条执行路径失去保证。

`++counter` 是最典型的反例：它是一次加载加一次存储，两个线程可以在两步之间交错，丢更新：

```cpp
// race_demo.cpp：故意的数据竞争，供 TSan 观察
int unsafe = 0;
for (int t = 0; t < 4; ++t)
    workers.emplace_back([&unsafe] {
        for (int i = 0; i < 10000; ++i) ++unsafe;
    });
// 期望 40000，实际经常更小——但任何值都不「保证」出现
```

肉眼review几乎不可能发现这类问题，正确工具是 **ThreadSanitizer**：

```bash
# GCC / Clang
g++ -g -O1 -fsanitize=thread race_demo.cpp -o race_demo && ./race_demo

# MSVC（VS 2022 起，实验特性，仅 x64）
cl /Zi /fsanitize=thread race_demo.cpp
```

TSan 输出一份 `data race` 报告，列出两条冲突访问的完整堆栈与相对时序。两个要点：

1. 即使某次运行碰巧输出 40000，竞争依然存在——报告看的是访问关系，不是运行结果。
2. 数据竞争演示程序**不进自动测试**：含 UB 的程序不能作为断言依据，本教程把它注册成独立可执行文件，仅供手动运行。

修复竞争的两条正路：互斥量（下一节）或原子变量（下一课）。选择依据是保护范围：一段「必须整体执行」的逻辑用 mutex；单个变量的独立更新用 atomic。

## 3. mutex 家族与 RAII 锁

`std::mutex` 保护的从来不是变量，而是**临界区**——一段访问共享状态的代码。锁必须用 RAII 包装，手动 `lock()/unlock()` 在异常路径上必死：

```cpp
std::lock_guard<std::mutex> lock(mutex);   // 构造加锁，析构解锁，不可手动操作
std::unique_lock<std::mutex> lock(mutex);  // 更重，但可延迟/移动/配合条件变量
std::scoped_lock lock(mutex_a, mutex_b);   // C++17：一次锁多个，内部防死锁
```

选择惯例：默认 `lock_guard`；需要条件变量或提前解锁时 `unique_lock`；一次锁多个互斥量用 `scoped_lock`。

## 4. 死锁与两种防法

死锁的经典形态是**环路等待**：线程 A 持有锁 1 等锁 2，线程 B 持有锁 2 等锁 1。两个防法：

1. **全局固定顺序**：所有代码都按同一顺序加锁（如先 inventory 后 quest）。
2. **`std::scoped_lock` 一次锁多个**：C++17 用内部死锁避免算法同时获取：

```cpp
std::scoped_lock lock(inventory_mutex, quest_mutex);   // 反向调用也不死锁
```

团队中无法约束所有人遵守顺序时（跨模块代码、第三方回调），`scoped_lock` 是更稳的选择。

## 5. 条件变量与虚假唤醒

条件变量解决「等某个条件成立再继续」：生产者放任务并 `notify`，消费者在无任务时睡眠：

```cpp
std::unique_lock<std::mutex> lock(mutex);
ready.wait(lock, [&] { return !tasks.empty() || producer_done; });
```

必须用**谓词版 `wait`**。等价展开是「循环 + 检查」：

```cpp
while (!pred()) cv.wait(lock);
```

原因有二：操作系统可能无缘故唤醒（虚假唤醒）；`notify` 与 `wait` 之间存在竞窗（通知在检查之前到达）。谓词版把这两件事一并防住。退出流程的惯例是「停止标志 + 队列排空」复合条件，保证收尾时不丢任务。

## 6. 游戏线程从哪里来

本课的线程都是「为一个任务现开的」，这类线程创建成本高（微秒级）、无法复用。游戏引擎的通行结构是**常驻 worker 线程池 + 任务系统**：线程启动一次，之后不断领取小任务。第三章（实战）会手写线程池，第四章（实战）把线程池升级成带依赖 DAG 与 work stealing 的任务系统——那是帧内并行的正确形态。

本课的 mutex/条件变量是所有这些设施的地基：任务队列靠它们实现，调度器的锁竞争分析也基于本课的共享状态模型。

## 7. 常见误区

- 忘记 join 或在异常路径上漏 join：RAII 守护解决。
- 认为 `++counter` 是原子的：它是加载加存储两步。
- 只锁「读」或只锁「写」一边：竞争需要双方都有同步，读写都进临界区。
- 无谓词地使用 `wait`：被虚假唤醒后拿到空队列直接崩溃。
- 在持锁状态下做耗时操作（IO、分配大内存、回调外部代码）：临界区要短。

## 8. 练习

1. 把 `JoinGuard` 的移动构造删掉，观察 `vector<JoinGuard>` 编译失败的原因，再恢复并解释移动语义在这里的作用。
2. 运行 `ch03_race_demo` 十次，记录输出分布；再用 TSan 运行一次，摘录报告中的两条冲突堆栈。
3. 把 `race_and_fix` 中 mutex 方案改成「只锁写、不锁读」，观察 TSan 或结果异常，说明为什么读也必须同步。
4. 用两个线程以相反顺序锁两把 mutex（不用 `scoped_lock`），加入打印观察死锁现场，然后改用 `scoped_lock` 修复。
5. 给生产者-消费者加一个「消费者处理耗时 1 ms」的模拟延迟，验证谓词版 `wait` 在队列空时不会崩溃。

## 9. 小结

- 线程析构前必须收束；RAII 守护让 join 不可遗漏。
- 数据竞争是 UB：结果「碰巧正确」不等于正确，TSan 看访问关系。
- 临界区用 RAII 锁：`lock_guard` 默认、`unique_lock` 配条件变量、`scoped_lock` 防死锁。
- 条件变量必须谓词等待，退出条件是「停止标志 + 队列排空」。
- 现开线程不适合帧内并行；线程池与任务系统是引擎的通行形态。

## 参考资料

- [cppreference：`std::thread`](https://en.cppreference.com/w/cpp/thread/thread)
- [cppreference：`std::condition_variable`](https://en.cppreference.com/w/cpp/thread/condition_variable)
- [cppreference：`std::scoped_lock`](https://en.cppreference.com/w/cpp/thread/scoped_lock)
- [MSVC：`/fsanitize` 系列文档](https://learn.microsoft.com/cpp/build/reference/fsanitize)
- [ThreadSanitizer（GCC/Clang）使用说明](https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual)
- [Critical Section：Threads, Mutexes, and Atomics](https://www.tedkoomen.com/blog/engineering/threads-mutexes-and-atomics/)
- 第三章调研与确认记录：[ch03-research](../resources/ch03-research.md)
