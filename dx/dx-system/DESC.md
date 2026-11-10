---
id: dx-system
title: dx 知识体系
brief: 定义 dx 知识单元本身：目录结构、frontmatter 规范、操作语义与索引机制
tags: [meta, system]
aliases: [dx, 知识单元]
father: null
---
# dx 知识体系

## 是什么
本工作区的基础知识组织单元。每个 dx 是一个文件夹，代表一个主题/流程/规范。

## 作用
为 agent 提供可渐进探索的上下文素材：索引给一句话线索，DESC 给概览，子目录文件给细节；agent 按需深入（见 skill `dx-explore`）。

## 与父级的关系
本 dx 是根单元（father: null）。

## 结构
| 组成 | 信任级别 | 内容 |
|---|---|---|
| `DESC.md` | — | frontmatter 元数据 + 描述正文，元数据唯一事实来源 |
| `knowledge/` | 可信 | 内部已确认的笼统知识（核心数据结构/流程/事实） |
| `reference/` | 不可信 | 外部网页/调研记录，带 `source`/`fetched` |
| `spec/`（可选） | 规范 | 分级操作规范，`level`: required/default/optional |

## 操作
创建/修改/删除由 skills `dx-create`、`dx-manage` 完成；所有 `INDEX.json` 由 `dx-manage/scripts/reindex.py` 从 DESC frontmatter + 文件系统重建。
