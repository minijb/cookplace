---
title: 第四章（中下）：游戏模式精选
series: cpp17-game-dev
chapter: 第四章（中下）
topic: Update Method、Observer/Event Queue、Object Pool 与 Spatial Partition
audience: 已掌握 ECS 架构、要组织游戏运行时行为的前端游戏开发工程师
outcome: 能实现并正确选用四个高频游戏模式，说明各自的时机与代价
environment: C++17；CMake 3.16+；Windows + MSVC 19.50 已验证；2026-11-18
date: 2026-11-18
---
# 第四章（中下）：游戏模式精选

适用环境：C++17、CMake 3.16 或更高版本；示例已在 Windows + MSVC 19.50 下验证。

你将学会：

- 用 Update Method 统一驱动异构游戏对象。
- 在 Observer 的立即通知与 Event Queue 的延迟分发之间做选择。
- 用对象池把热路径上的分配降到零。
- 用均匀网格把近邻查询从 O(n²) 降到近似线性，并防住对角格漏检。

完整代码位于 [游戏模式示例](../code/14-game-patterns/main.cpp)。模式定义出自 Game Programming Patterns（Nystrom，在线免费），本课用 C++17 重写并接上确定性断言。

## 1. Update Method：每帧驱动的统一形态

主循环（第四章上）不该认识每一种对象；Update Method 让对象自己知道每帧做什么，循环只负责遍历调用：

```cpp
struct GameObject {
    virtual ~GameObject() = default;
    virtual void update(double dt) = 0;
};

for (auto& object : world) {
    object->update(1.0 / 60.0);
}
```

敌人走位、飘字倒计时、粒子衰减各自实现 `update`，增删对象类型不动主循环。与 ECS 的关系要说清：Update Method 是**驱动结构**，ECS 是**数据布局**——ECS 系统循环本质上就是「去掉虚调用的 Update Method」。原型期用虚函数版完全合理，实体上量后再换系统批量遍历。

代价提醒：虚调用打断内联、对象列表是指针追逐。它买的是灵活与解耦，性能敏感的海量实体换 ECS（第四章中）。

## 2. Observer 与 Event Queue：通知的两种时机

游戏里到处是「某事件发生后，多方要反应」：敌人死亡 → 加分、播动画、掉落。直接调用把发送方和所有反应焊死；Observer 解耦成「订阅」：

```cpp
void on_event(GameEvent event) {
    if (event == GameEvent::EntityFell) {
        fell_unlocked_ = true;   // 成就系统只订阅、不被感知
    }
}
```

**立即通知**（经典 Observer）响应最快，但订阅者的回调可能反过来修改场景——在物理遍历中改世界是经典的崩溃源。**Event Queue** 把事件先入队，选定安全时机（帧末、系统边界）再统一分发：

```cpp
event_queue.push(GameEvent::EntityFell);
// ……此刻成就尚未解锁：事件只是被记录
REQUIRE(!achievements.fell_unlocked());

while (!event_queue.empty()) {          // 帧末 flush
    achievements.on_event(event_queue.front());
    event_queue.pop();
}
REQUIRE(achievements.fell_unlocked());
```

示例明确断言了这个时序差异。选择原则：回调不会改遍历中的世界 → 立即通知；可能改 → 入队延迟。队列还有第二个红利：把「每帧几十次零散通知」合并成一次批量处理。代价是一帧内的分发延迟。

## 3. Object Pool：热路径零分配

粒子、弹幕、伤害数字每帧成百上千地生灭，直接 new/delete 会把帧预算耗在分配器与内存碎片上。对象池一次分配固定槽位，之后循环复用：

```cpp
Particle* spawn() {
    for (std::size_t i = 0; i < slots_.size(); ++i) {
        Particle& slot = slots_[(next_ + i) % slots_.size()];
        if (!slot.active) {
            slot.active = true;
            next_ = (next_ + i + 1) % slots_.size();
            return &slot;     // 复用旧槽位：地址不变、分配次数不变
        }
    }
    return nullptr;           // 池满拒绝，绝不临时扩容
}
```

示例断言了两条核心性质：池满时返回 `nullptr`（拒绝策略明确，不做隐藏扩容）；释放后 `spawn` 拿回**同一个地址**。第二条正是池的意义——地址稳定让「持指针的粒子引用」不失效，也让缓存长期留在热槽位上。

> ⚠️ 池化对象的生命周期由池管理，绝不能再被 `unique_ptr` 或 delete 触碰——「谁分配谁释放」的所有权规则在池语境下是「池分配、池回收（kill）」。

## 4. Spatial Partition：按位置组织查询

「找出半径 2 米内的敌人」这类查询，朴素做法是全对全比较 O(n²)。Spatial Partition 把空间本身组织成数据结构，查询只看附近的格子。最小实现是均匀网格：

```cpp
// 插入：按坐标算格子
cells_[cell_index(points[i])].push_back(i);
// 查询：同格 + 相邻格
```

示例在 1000 个随机点上数「距离 ≤ 2」的点对，网格结果与暴力一致、耗时约为其八分之一：

```text
pairs=627 brute=6186 us, grid=812 us
```

两个工程要点：

1. **对角格必须查**：只查右、下两个方向是常见错误——对角格里的两点距离可以小于半径，漏配对。全量方向有 8 个，用「右、下、右下、左下」4 个方向即可两两覆盖全部跨格配对且不重复计数。
2. **格子尺寸 ≈ 查询半径**：一查 9 格（3×3），每格平均几个候选。格太大退化回暴力，格太小格子管理成本反超收益。

网格适合「对象多、查询频繁、世界平坦」的场景；对象移动后只需更新所在格。树形结构（四叉树/八叉树）适合尺度差异大的稀疏空间——先问查询频率，再选结构。

## 5. 何时不用模式

Game Programming Patterns 的基调是克制，四个模式各有一条「别用」的红线：

- Update Method：对象类型很少时，虚调用层是纯开销，直接写循环。
- Event Queue：一帧内的事件量很小、订阅者固定时，直接调用更可读。
- Object Pool：对象生灭频率低时，池是徒增复杂度的固定内存占用。
- Spatial Partition：对象少或查询稀疏时，O(n²) 反而更快（常数小、无结构维护成本）。

判断依据永远是实测的查询频率与对象规模，不是「引擎该有这个模块」。

## 6. 常见误区

- 用 Update Method 处理海量同类实体：虚调用 + 指针追逐，应换 ECS 系统遍历。
- 订阅者在立即回调里销毁/添加对象：改用 Event Queue 延迟到帧末。
- 池化对象被外部 delete：所有权只在池内。
- 网格漏查对角格：配对数对不上暴力基准。
- 用模式替代测量：先写基准断言（如示例的对拍），再谈优化。

## 7. 练习

1. 给 Update Method 示例加删除逻辑：`FloatingText` 生命耗尽后安全移出 `world`，注意遍历中删除的坑。
2. 把 Event Queue 改成固定容量环形队列，验证池满时的丢弃策略与新事件优先级。
3. 用自定义分配器统计粒子池运行 60 秒的堆分配次数，应为常量。
4. 把网格尺寸从查询半径改成 0.5 倍与 2 倍，实测对拍耗时曲线，解释格子尺寸的权衡。
5. 找你当前项目里一处「每帧 O(n²) 距离判定」，估算改网格后的候选数下降比例。

## 8. 小结

- Update Method 是驱动结构，海量同类实体换 ECS 系统遍历。
- 立即通知快但危险，事件队列用一帧延迟换安全时机。
- 对象池把热路径分配降为零，地址稳定、拒绝扩容。
- 网格按位置组织查询，对角格必须覆盖，格子尺寸约等于查询半径。
- 模式由查询频率与对象规模决定，不由架构癖好决定。

## 参考资料

- [Game Programming Patterns：Update Method](http://gameprogrammingpatterns.com/update-method.html)
- [Game Programming Patterns：Observer / Event Queue / Object Pool / Spatial Partition](http://www.gameprogrammingpatterns.com/contents.html)
- [Generalist Programmer：Game Programming Patterns 完整目录](https://generalistprogrammer.com/game-design-patterns)
- 第四章调研与确认记录：[ch04-research](../resources/ch04-research.md)
