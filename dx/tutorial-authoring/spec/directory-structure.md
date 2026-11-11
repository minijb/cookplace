---
title: 教程目录结构规范
brief: 教程文件夹按规模二选一：小主题用编号文件式（A），大主题用部-章-课三层（B），主题杂项放 misc/，强制执行
level: required
---
# 教程目录结构规范

## 模式选择

| 规模 | 判定标准 | 采用结构 |
|---|---|---|
| 小主题 | 围绕单一技能点，章节 ≤ 8 篇，可一次性完成 | 模式 A：编号文件式 |
| 大主题 | 多阶段学习路线，章节 > 8 篇，或计划连载扩展 | 模式 B：部-章-课三层 |

判定拿不准时，从小主题（模式 A）起步；章节超过 8 篇后升级为模式 B，升级时重排目录并同步更新所有内部链接。

## 模式 A：编号文件式（小主题）

```
tutorials/<NN-主题-slug>/
├── README.md            # 教程入口：简介、你将学会什么、章节目录
├── 01-setup.md          # 章节文件：两位序号 + kebab-case
├── 02-install.md
├── 03-core-steps.md
├── assets/              # 本教程全部图片
│   └── 01-architecture.png
└── code/                # 可运行的示例代码
    └── demo.py
```

## 模式 B：部-章-课三层（大主题）

```
tutorials/<系列-slug>/
├── README.md            # 系列入口：定位、读者画像、路线图
├── part-01-basics/      # 部：两位序号 + kebab-case
│   ├── 01-introduction/
│   │   ├── index.md     # 课：入口文件固定 index.md
│   │   └── assets/
│   └── 02-first-project/
│       └── index.md
└── part-02-advanced/
    └── 01-cli/
        └── index.md
```

## 通用规则（两种模式都必须遵守）

1. 一个教程一个独立文件夹，文件夹名用 kebab-case 英文；中文标题写进文件正文或 frontmatter，不进文件夹名。
2. 章节、部均用两位序号前缀（`01-`、`02-`…）表达阅读顺序；序号只用于排序，不进正文标题。
3. 教程入口文件固定为 `README.md`；模式 B 中每「课」的入口文件固定为 `index.md`。
4. 图片集中放 `assets/`，文件名带所属章节序号（如 `01-architecture.png`），便于与正文对应。
5. 可运行代码集中放 `code/`，参考资料与附属材料放 `resources/`；`code/` 内代码必须可独立运行，README 说明运行方式。
6. 目录顺序与正文目录一一对应；发布平台的导航排序以序号为准。
7. 草稿、废弃章节不得进入正式目录。

模式来源与调研记录见 `dx:tutorial-authoring/reference/tutorial-structure-sources.md`。

## 杂项目录（主题级 misc）

杂项指围绕某主题、但构不成教程的短内容：小技巧、踩坑记录、代码片段、环境备忘等。统一放 `misc/<主题-slug>/`，与 `tutorials/` 平级：

```
misc/
├── docker/
│   ├── README.md          # 主题杂项索引：每条一行说明 + 指向同主题教程
│   ├── 01-tip-proxy.md    # 每条杂项一个文件：两位序号 + kebab-case
│   ├── 02-pitfall-port.md
│   └── snippets/          # 代码片段（可选）
└── python/
    └── …
```

规则：

1. 每条杂项一个文件，只讲一个点，篇幅不超过 100 行；无章节结构，不套用教程模板。
2. 条目文件用两位序号 + kebab-case 命名，命名体现类型：`01-tip-…`（技巧）、`02-pitfall-…`（踩坑）、`03-snippet-…`（片段）、`04-setup-…`（环境备忘）。
3. `README.md` 为主题杂项索引：逐条一行说明；同主题的教程与杂项互相链接。
4. 代码片段集中放 `snippets/`，其余短文档直接放主题文件夹根部。
5. 杂项正文仍须遵守排版规范 `dx:tutorial-authoring/spec/layout.md`；涉及抽象概念时按 `dx:tutorial-authoring/spec/diagrams.md` 用 mermaid 绘制。
6. 升级规则：同主题杂项超过 10 条，或单条内容膨胀到需要章节划分时，升级为教程（模式 A）——新建教程文件夹迁移内容，杂项 README 中改为指向新教程。
