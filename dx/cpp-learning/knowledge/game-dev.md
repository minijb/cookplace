---
title: 游戏开发专题
brief: 游戏循环、ECS 架构、对象生命周期与游戏多线程
tags: [cpp, gamedev]
---
# 游戏开发专题

## 游戏循环（Game Loop）
- 每帧三件事：处理输入（不阻塞）→ 更新状态 → 渲染；用时间差控制节奏。
- 固定步长更新 + 渲染插值可换取物理确定性；变步长简单但不确定。
- 实测确认（教程 part-04）：浮点秒数累加器会漂移掉步（60 步被吃成 59 步），整数刻度累积精确无偏——引擎通行做法。帧时间钳制防死亡螺旋（0.5s 尖峰钳到 0.25s 补 15 步）；插值状态只服务渲染，模拟永不读。
- 与前端的关系：Web 游戏主循环跑在 `requestAnimationFrame` 上，帧调度权在浏览器。

## 架构模式
- ECS（Entity-Component-System）：组合优于继承；组件即数据、系统即批量处理——与缓存友好天然契合，是数据导向设计的代表。
- 实测确认（教程 part-04，含手写 sparse set 迷你 ECS 与 EnTT v3.13.2 vendored 实测）：实体 = 索引 + 代数，销毁递增代数使旧句柄可检测失效；遍历中不得增删组件（swap-remove 打乱迭代），先收集后销毁；⚠ EnTT `create(hint)` 传过期句柄会把过期句柄原样复活（valid 变 true），只用于快照恢复类场景。10 万实体 × 60 帧 view 移动约百 ms 量级。
- 高频模式（出自 Game Programming Patterns）：Observer（事件分发）、Object Pool、Flyweight、Update Method、Dirty Flag、Spatial Partition。实测：均匀网格必须覆盖对角格（右/下/右下/左下四方向两两去重），否则配对数对不上暴力基准。

## 生命周期与多线程
- 场景/关卡加载卸载有严格顺序；资源句柄化，不裸持指针跨帧。
- 主线程渲染 + 工作线程异步任务系统；数据竞争高发区：共享容器、单例、全局状态。任务系统专题见 `dx:cpp-learning/knowledge/job-system.md`，UE 侧落地见 `dx:cpp-learning/knowledge/unreal-engine.md`。

## 待深入
- 网络同步（帧同步/状态同步）、物理引擎集成、脚本绑定（Lua/JS）
