---
id: tutorial-authoring
title: 教程制作规范
brief: 规范教程的制作流程、排版、目录结构、图表绘制与资料查找方法
tags: [tutorial, process, formatting, research]
aliases: [教程, 教程制作, 教程规范, 教程排版, 写教程, 查资料, 目录结构, 杂项]
father: null
---
# 教程制作规范

## 是什么
规范「如何制作一篇教程」的知识单元，覆盖六个方面：
- 制作流程：从选题到发布维护的阶段划分与各阶段产出（见 `dx:tutorial-authoring/knowledge/production-pipeline.md`）。
- 选题门禁：生成教程前必须上网详细调研、与用户确认学习范围与深入点、按网络推荐补充章节并二次确认（见 `dx:tutorial-authoring/spec/research-confirm.md`）。
- 基础排版：教程正文的结构、格式与中英文混排约定（见 `dx:tutorial-authoring/knowledge/layout-principles.md`）。
- 资料查找：上网检索、可信度分层与交叉核实方法（见 `dx:tutorial-authoring/knowledge/web-research-methods.md`）。
- 目录结构：教程文件夹 A/B 模式与主题杂项 misc/ 归档（spec 见 `dx:tutorial-authoring/spec/directory-structure.md`，调研来源见 `dx:tutorial-authoring/reference/tutorial-structure-sources.md`）。
- 图表绘制：关系、时序、类结构等抽象概念用 mermaid 绘制（见 `dx:tutorial-authoring/spec/diagrams.md`）。

## 作用
制作、审校或协助他人制作教程时，按本 dx 的 spec 执行：
- `dx:tutorial-authoring/spec/production-pipeline.md`（流程，default）
- `dx:tutorial-authoring/spec/research-confirm.md`（选题调研与确认门禁，required）
- `dx:tutorial-authoring/spec/layout.md`（排版硬规则，required）
- `dx:tutorial-authoring/spec/directory-structure.md`（目录结构与杂项归档，required）
- `dx:tutorial-authoring/spec/diagrams.md`（图表绘制，default）
- `dx:tutorial-authoring/spec/web-research.md`（查资料，default）

## 与父级的关系
本 dx 是根单元（father: null）。

## 与子级的关系
子 dx：`cpp-learning`（C++ 学习，前端游戏开发方向）——将本 dx 的规范落地到具体学习领域。若「排版」或「资料检索」某一方面内容膨胀，也可拆分为子 dx（如 tutorial-layout、web-research）。
