---
title: 游戏开发专题
brief: 游戏循环、ECS 架构、对象生命周期与游戏多线程
tags: [cpp, gamedev]
---
# 游戏开发专题

## 游戏循环（Game Loop）
- 每帧三件事：处理输入（不阻塞）→ 更新状态 → 渲染；用时间差控制节奏。
- 固定步长更新 + 渲染插值可换取物理确定性；变步长简单但不确定。
- 与前端的关系：Web 游戏主循环跑在 `requestAnimationFrame` 上，帧调度权在浏览器。

## 架构模式
- ECS（Entity-Component-System）：组合优于继承；组件即数据、系统即批量处理——与缓存友好天然契合，是数据导向设计的代表。
- 高频模式（出自 Game Programming Patterns）：Observer（事件分发）、Object Pool、Flyweight、Update Method、Dirty Flag、Spatial Partition。

## 生命周期与多线程
- 场景/关卡加载卸载有严格顺序；资源句柄化，不裸持指针跨帧。
- 主线程渲染 + 工作线程异步任务系统；数据竞争高发区：共享容器、单例、全局状态。任务系统专题见 `dx:cpp-learning/knowledge/job-system.md`，UE 侧落地见 `dx:cpp-learning/knowledge/unreal-engine.md`。

## 待深入
- 网络同步（帧同步/状态同步）、物理引擎集成、脚本绑定（Lua/JS）
