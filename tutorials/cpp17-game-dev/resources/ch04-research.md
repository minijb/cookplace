---
title: 第四章调研与确认记录
series: cpp17-game-dev
topic: 游戏循环、ECS、游戏模式、对象生命周期与任务系统的教程调研
audience: 教程作者与审校者
outcome: 追溯第四章的范围、章节依据、权威来源和代码验证状态
environment: 在线资料调研；2026-11-18
date: 2026-11-18
---
# 第四章调研与确认记录

## 范围确认

- 初始路线：`dx:cpp-learning/knowledge/roadmap.md` 第四阶段，以及 `game-dev.md`、`job-system.md`、`async.md`。
- 用户确认 ECS 方式（2026-11-18）：「手写为主 + 指路」与「引入 EnTT」合并执行——先零依赖手写带 sparse set 思想的迷你 ECS，再用 vendored EnTT v3.13.2（单目录头文件库，`code/third_party/entt/`）写对照示例，文末指路 Flecs。
- 用户确认附加专题（2026-11-18）：游戏模式精选（Update Method、Observer/Event Queue、Object Pool、Spatial Partition）与对象生命周期与句柄化全部纳入。
- 沿用环境：C++17、跨平台 CMake，示例接入 CTest；EnTT 以 vendored 源码接入，不引入配置期网络依赖。
- 经确认的章节结构：游戏循环与时间 → 实体架构与 ECS（手写 + EnTT 对照）→ 游戏模式精选 → 对象生命周期与句柄化 → 任务系统实战，共五课。

## 调研要点

1. 游戏循环的公认经典是 Glenn Fiedler「Fix Your Timestep!」：固定步长模拟 + 累加器 + 帧时间钳制 + 渲染插值；多个近期教程（Comfy Engine、fromscratchlab、shipthatcode）沿用同一结构。插值只服务渲染，游戏逻辑不得读插值状态。
2. 变步长的核心问题是物理不确定性与数值漂移；固定步长的代价是需要插值与螺旋死亡防护（帧时间钳制）。
3. ECS 教学（Bevy、flecs FAQ、studyplan.dev、skypjack 系列）一致强调：实体是轻量 ID、组件是纯数据、系统是批量函数；收益来自平坦数组、预取友好与可并行调度。sparse set（EnTT）与 archetype（Flecs/Unity DOTS）是两条主流存储路线，跨组件迭代时只有首个（或分组后全部）组件保证内存连续。
4. 游戏模式（Game Programming Patterns）：Update Method、Observer/Event Queue、Object Pool、Spatial Partition 是高频四件套；Spatial Partition 的收益取决于查询频率与对象移动频率的比值。
5. 句柄化资源（generation handle）是游戏行业通行做法：索引 + 代数双检，检测悬垂访问；引擎常用实现（如 Handles. 资源系统）均属此模型。教程演示代数递增使旧句柄失效的最小实现。
6. 帧分配器（linear/arena）：整帧一次性重置，禁止逐对象释放；与「任务内禁止无界分配」纪律（`dx:cpp-learning/knowledge/job-system.md`）直接衔接。
7. 任务系统教学（Mighty Professional、rismosch、Intel GTS、Unity ParallelFor 文档）：worker 每线程双端队列，本端 LIFO、他端偷取 FIFO；依赖 DAG + fork/join 是通用骨架。教学实现可用互斥队列近似，正文注明生产实现是 Chase-Lev 式无锁队列。
8. 同步点纪律：游戏线程只在 join 后读权威状态；等待损害扩展性，优先用依赖建图而非显式 Wait（UE 官方文档同口径）。

## 来源与核实标记

| 来源 | 用途 | 核实状态 |
|---|---|---|
| [Glenn Fiedler：Fix Your Timestep!](https://gafferongames.com/post/fix_your_timestep/) | 固定步长 + 累加器 + 插值原型 | 已检索，2026-11-18 |
| [Game Programming Patterns：Game Loop / Update Method / Observer / Object Pool / Spatial Partition / Event Queue](http://www.gameprogrammingpatterns.com/contents.html) | 模式定义与游戏示例 | 已检索，2026-11-18 |
| [Comfy Engine：Fixed timestep game loop](https://comfyengine.org/blog/fixed-timestep-game-loop/) | 累加器/钳制/插值的现代实现细节 | 已检索，2026-11-18 |
| [Bevy：ECS 概念](https://bevy.org/learn/quick-start/getting-started/ecs/) | ECS 三概念的主流表述 | 已检索，2026-11-18 |
| [flecs：Entity Component System FAQ](http://www.flecs.dev/ecs-faq/) | ECS 特征清单与常见问题 | 已检索，2026-11-18 |
| [EnTT：Crash Course 与 Wiki](https://skypjack.github.io/entt/md_docs_2md_2entity.html) | sparse set 模型、registry/view 用法 | 已检索，2026-11-18；EnTT v3.13.2 以 vendored 源码实测 |
| [studyplan.dev：Implementing ECS in C++](https://www.studyplan.dev/ecs/entity-component-system) | C++ 手写 ECS 的章节组织 | 已检索，2026-11-18 |
| [gamedev.stackexchange：EnTT 迭代与 grouping](https://gamedev.stackexchange.com/questions/184321/efficient-communication-between-entities-using-ecs-via-entt) | 跨组件迭代连续性 caveat | 已检索，2026-11-18 |
| [Mighty Professional：Job Systems from Scratch](https://mightyprofessionalgaming.com/tutorials/job-systems-from-scratch.html) | 从锁队列到 work stealing 的教学路径 | 已检索，2026-11-18 |
| [rismosch：Building a JobSystem](https://www.rismosch.com/article?id=building-a-job-system) | 每 worker 双端队列与偷取方向 | 已检索，2026-11-18 |
| [Intel GTS：Terminology 与技术文章](https://gametechdev.github.io/GTS-GamesTaskScheduler/documentation/html/dev_guide_prog_mod_terminology.html) | work stealing 术语与负载均衡定位 | 已检索，2026-11-18 |
| [Unity：Parallel jobs 文档](https://docs.unity.com/en-us/engine/6000.6/manual/scripting/programming-distribute-work-threads/job-system/parallel-for-jobs) | ParallelFor 分批与偷半策略的工业参照 | 已检索，2026-11-18 |
| [UE：Tasks Systems in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/tasks-systems-in-unreal-engine) | 依赖 DAG、优先依赖建图不显式等待 | 已检索，2026-11-18 |

## 验证状态

- 教程代码目录 `code/12-game-loop` 至 `code/16-job-system` 与 `code/third_party/entt/`（EnTT v3.13.2 vendored，无网络依赖），已随写作全部构建并接入 CTest。
- 2026-11-18：Windows + MSVC 19.50 干净构建零警告，全部用例（含 `ch04_entt_demo`）通过。实测确认：浮点秒数累加器漂移掉步（改整数刻度）；EnTT `create(过期 hint)` 会复活过期句柄（已写入教程警示）；均匀网格必须覆盖对角格。
