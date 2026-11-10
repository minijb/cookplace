---
title: 命名与结构规范
brief: dx id/文件夹/文件的命名与目录结构的强制规则
level: required
---
# 命名与结构规范

- `id` = 文件夹名，必须匹配 `^[a-z0-9]+(-[a-z0-9]+)*$`（kebab-case），全工作区唯一；中文放 `title`。
- 每个 dx 必须有 `DESC.md`，且 frontmatter 的 `id` 与文件夹名一致。
- dx 内子目录只允许 `knowledge/`、`reference/`、`spec/`；dx 根下除 `DESC.md` 外不放任何文件。
- 所有知识文件为 `.md` 且带 frontmatter（必填字段见 `dx:dx-system/knowledge/frontmatter.md`）。
- 子文件名用 kebab-case，一个主题一个文件；spec 一个 level 一个文件（frontmatter 只能存一个 level）。
- `INDEX.json` 为脚本生成物，禁止手工编辑。
