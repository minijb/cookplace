---
title: Unreal Engine 基础
brief: UE C++ 子集：对象模型与反射、GC、Gameplay 框架、容器与智能指针、线程模型与任务系统
tags: [cpp, ue, gamedev]
---
# Unreal Engine 基础

UE 是「带大型运行时的 C++ 框架」：标准 C++ 之上加了反射、GC 与一套 Gameplay 架构。官方文档站 dev.epicgames.com/documentation，来源清单见 `dx:cpp-learning/reference/learning-sources.md`。

## 对象模型与反射
- UObject 是根类型；`UCLASS()` / `USTRUCT()` / `UFUNCTION()` / `UPROPERTY()` 宏把类型注册进反射系统——由 UnrealHeaderTool 生成代码实现，不是运行时反射。
- 反射支撑四大系统：编辑器细节面板、序列化、垃圾回收、网络复制，以及蓝图互操作。
- 只有反射属性（UPROPERTY，或 UE 容器内的对象引用）才被 GC 感知。

## 垃圾回收
- GC 基于可达性分析：从根集出发，不可达 UObject 被销毁；官方文档明确 GC 在游戏线程上运行。
- 销毁流程三段式：BeginDestroy → IsReadyForFinishDestroy → FinishDestroy，为渲染线程异步释放资源留窗口（先排队释放命令，围栏确认后才真正删内存）。
- 裸指针持有的 UObject 引用引擎不知道：不自动置空、不防回收（官方 Unreal Object Handling 口径）。
- 观察用 `TWeakObjectPtr`：不阻止回收，可检测失效。

## 智能指针两套体系
- UObject 相关：`TWeakObjectPtr` / `TObjectPtr`（配合 UPROPERTY）。
- 非 UObject：`TSharedPtr` / `TSharedRef` / `TWeakPtr`——官方 Unreal Smart Pointer Library 是 C++11 智能指针的自研实现；`TSharedRef` 是**非空版本**的 shared pointer。

## 容器与字符串
- TArray（动态数组，同 vector 语义）/ TMap / TSet / TQueue。
- FString（可变字符串）、FText（本地化显示）、FName（全局字符串池、比较 O(1)，热路径常用）。

## Gameplay 框架层级
- GameInstance（进程级）→ World → GameMode（规则，仅服务器）/ GameState → Controller（PlayerController / AIController）→ Pawn / Character → Actor → Component（可组合挂载）。
- 生命周期：Actor 的 BeginPlay / Tick / EndPlay；Component 组合是 ECS 思想的工程化近似。

## 线程模型
- GameThread 权威：官方文档明确「game thread owns all AActor / UObject state」，Tick、蓝图执行都在此；渲染线程晚 1–2 帧，游戏线程在 Tick 末最多等渲染线程追平两帧；RHI 提交独立成线程（UE4.22 起）；并行走 TaskGraph worker 池。
- 跨线程纪律：非游戏线程不能调用多数 UWorld/Actor 接口，渲染线程永不直接解引用 UObject 指针（用 SceneProxy 镜像数据或 fence 保护）；回游戏线程用异步委托或 ENQUEUE_UNIQUE_RENDER_COMMAND。
- 并行工具：TaskGraph（上一代 job manager）与 Tasks System（UE5，官方定位「improves on the TaskGraph」，两者共用同一调度后端）；Tasks System 支持 prerequisites 依赖 DAG（依赖不阻塞 worker）、嵌套任务（父任务随嵌套全部完成）、Pipe 串行链（近似 green thread）、五档优先级；UE5.5 起以 oversubscription（唤醒 standby 线程）替代 busy-waiting。设计原则见 `dx:cpp-learning/knowledge/job-system.md`。

## 构建与模块
- 一切皆模块（`*.Build.cs`）；Target.cs 定义构建目标；插件 = 带 .uplugin 的模块集合。
- 蓝图 / C++ 边界：`BlueprintCallable` 暴露 C++ 给蓝图；`BlueprintImplementableEvent` 让蓝图实现逻辑。

## 待深入
- GameplayAbilitySystem（技能系统）、网络复制（Replication）、Subsystems、移动与物理组件、Lumen/Nanite 概念

## 复核记录
- 2026-11-10 对照官方文档逐条核对：Programming with C++、Reflection System、Tasks Systems、Threaded Rendering 四页，结论与官方口径一致，版本基线 UE5.x。
- 「GameMode 仅服务器」出自官方 Gameplay Framework 口径，未单独抓取原文，建议实戹时再验证；GC 三段式销毁流程出自 Threaded Rendering 页的 USkeletalMesh 示例。
