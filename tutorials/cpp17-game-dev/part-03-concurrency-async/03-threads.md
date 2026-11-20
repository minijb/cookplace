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

### 1.1 `JoinGuard` 逐行拆解

**构造**——变参成员模板，序号对应注释：

```cpp
class JoinGuard {
public:
    template <typename F, typename... Args>                // ① 成员函数模板
    explicit JoinGuard(F&& entry, Args&&... args)          // ② explicit + 转发引用
        : thread_(std::forward<F>(entry),                  // ③ 成员初始化列表
                  std::forward<Args>(args)...) {}          //    逐个转发，直接启动线程
```

① 模板标在构造函数上而不是类上：`JoinGuard` 对任何可调用物（lambda、函数指针、仿函数）都保持同一个类型，不会产生 `JoinGuard<LambdaA>`、`JoinGuard<LambdaB>` 这样的异类——存进 `vector<JoinGuard>`（示例代码正是这么用的）不挑元素类型。

② `explicit` 禁止隐式两步转换，只允许 `JoinGuard g(lambda);` 直接初始化，不允许 `JoinGuard g = lambda;`。`F&&` 与 `Args&&...` 是转发引用：既能绑左值也能绑右值，配合 `std::forward` 保住实参的值类别（右值触发移动，左值保持拷贝），语法就是 3 章（上）参数包一节的形状。

③ 成员初始化列表把转发来的实参直接喂给 `thread_` 成员的构造函数——这也是唯一的选择：**`std::thread` 只能靠构造函数启动执行流**，不存在「先默认构造、再赋一个函数」的 API。这也是守护类用变参构造代为创建线程、而不是接收现成 `std::thread` 的原因。构造完成的瞬间，线程已经在跑了。

**析构**——RAII 的收口点：

```cpp
    ~JoinGuard() {                                  // ④ 所有退出路径的必经之路
        if (thread_.joinable()) {                   // ⑤ 还没收束才需要 join
            thread_.join();                         // ⑥ 阻塞等待执行流结束
        }
    }
```

④ 析构函数是 RAII 的收口点：正常离开作用域、提前 `return`、异常展开，全部经过这里。手工 join 容易忘的路径，这里兑底。

⑤ `joinable()` 为 `true` 表示这个对象「代表一个尚未收束的执行流」。实测（MSVC 19.50）为 `false` 的四种情形：默认构造的空线程、已经 join 过、已经 detach 过、被移动走之后。if 覆盖这些情形，析构才能无脑安全。

⑥ `join()` 阻塞当前线程，直到执行流结束，然后与对象脱钩；只能成功一次。

**拷贝与移动**——所有权语义：

```cpp
    JoinGuard(const JoinGuard&) = delete;            // ⑦ 禁止拷贝构造
    JoinGuard& operator=(const JoinGuard&) = delete; // ⑧ 禁止拷贝赋值
    JoinGuard(JoinGuard&& other) noexcept            // ⑨ 移动构造：所有权转移
        : thread_(std::move(other.thread_)) {}
private:
    std::thread thread_;                             // ⑩ 唯一成员：被守护的资源
};
```

⑦⑧ 两个守护对象不能拥有同一个执行流——否则两个析构各 join 一次，第二次对已收束的线程调 join 会抛异常。`= delete` 把这类误用挡在编译期。

⑨ 移动构造把 `thread_` 的所有权转给新对象；实测被移走的 `other.thread_` 变成空线程、`joinable()` 为 `false`，其析构函数安全地什么都不做。`noexcept` 必不可少：`vector` 扩容搬家只在移动操作带 noexcept 时才敢用移动，否则退回拷贝——而拷贝已被 delete，直接编译失败。示例代码的 `vector<JoinGuard>` 正靠这一行撑起来的。

⑩ 守护类只持有一个成员：被守护的资源本身。类不需要额外状态，只负责「析构时把执行流收束」。

用起来只有几行（示例代码的两种用法）：

```cpp
JoinGuard worker([&result, &result_mutex] { /* ... */ });   // 构造：线程立即启动
// ...主线程继续做自己的事...
}                                     // 作用域结束：析构 → join

std::vector<JoinGuard> workers;       // 一组线程
workers.emplace_back([&guarded, &mutex] { /* ... */ });
}                                     // vector 析构 → 每个 JoinGuard 析构 → 逐个 join
```

两点实测边界（MSVC 19.50）：类没有移动赋值——析构函数与已删除的拷贝赋值都会阻止移动赋值的隐式生成，`g2 = std::move(g1)` 编译不过（`error C2280：尝试引用已删除的函数`）。对守护类这是可接受的限制：构造后不需要换绑线程。

这与第一章 RAII 的思路完全一致：资源（这里是「一个尚未收束的执行流」）的生命周期绑进对象——构造即启动，析构即收束。

> ⚠️ `join()` 只能调用一次；对不可 join 的线程（已 join、已 detach、空线程）调用会抛 `std::system_error`（错误条件 `invalid_argument`，注意不是 `std::invalid_argument` 类型本身）。判断依据是 `joinable()`，不是「我觉得它结束了」。

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
