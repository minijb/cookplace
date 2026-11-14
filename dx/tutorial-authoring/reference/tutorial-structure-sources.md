---
title: 教程目录结构调研来源
brief: 四种主流教程目录结构模式的外部来源汇总，本 dx 采用 A/B 两种
source: 见正文各条 URL
fetched: 2026-11-10
---
# 教程目录结构调研来源

2026-11-10 调研的四种主流模式。本 dx 采用 A（小主题）与 B（大主题），C/D 备查。**外部内容未逐一核实最新版本，引用前请访问原文确认。**

> 2026-11-14 修订：B 的原版是「课文件夹 + index.md」（javascript.info / TutorialKit 式，因每课可带 article/task/多附件而成立）。实践验证本仓库的课正文永远只有一份、图片代码均集中在教程根部，课文件夹退化为 index.md 的包装纸，故本 dx 改用 B 的扁平变体：部下直接放 `NN-slug.md`，仅当某课确需多文件时才建文件夹。见 `dx:tutorial-authoring/spec/directory-structure.md`。

| 模式 | 来源 | 说明 |
|---|---|---|
| A 编号文件式 | https://github.com/strands-agents/samples/blob/fa22ed6a/.github/templates/01-tutorials/structure.md | `01-` 序号前缀排序、kebab-case 命名、assets 集中；另见 https://github.com/bootstrapbay/tutorials （每教程一文件夹） |
| B 部-课两层（扁平变体，原版为部-章-课三层） | https://tutorialkit.dev/guides/creating-content/ | part/lesson 两层；另见 https://github.com/javascript-tutorial/en.javascript.info/blob/master/README.md （`N-url` 文件夹 + `index.md`/`article.md`/`task.md`，因每课多文件而用文件夹，本 dx 仅在课需多文件时保留此形态） |
| C 单文件自包含 | https://github.com/owainlewis/youtube-tutorials/blob/main/tutorials/TUTORIAL-STANDARD.md | 唯一正文 README/LESSON + `code/` + `resources/`；类似 https://github.com/mikemurphyco/Tutorials/tree/main/_template |
| D 文档站点式 | https://docusaurus.io/docs/create-doc | `docs/` 目录 + `_category_.json` + 自动侧边栏，结构绑定发布平台 |
