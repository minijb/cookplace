---
title: frontmatter 规范
brief: 各类 md 文件 frontmatter 的必填/可选字段定义
---
# frontmatter 规范

所有知识文件均为 .md + 精简 frontmatter（YAML 子集：`key: value`、`[a, b]` 列表、引号字符串、null/bool）。

| 文件 | 必填 | 可选 |
|---|---|---|
| `DESC.md` | `id`（=文件夹名，kebab-case）、`title`、`brief`、`father`（根为 null） | `tags`、`aliases`（用户常用说法列表，进 lookup） |
| `knowledge/*.md` | `title`、`brief` | `tags` |
| `reference/*.md` | `title`、`brief` | `source`（原始 URL）、`fetched`（YYYY-MM-DD）——均可选但强烈建议 |
| `spec/*.md` | `title`、`brief` | `level`（required/default/optional，缺省 default）、`tags` |

约定：
- `brief` 一句话，是索引里唯一展示的描述；写好 brief = 索引可读。
- `id` 只能 kebab-case（全小写英文/数字/连字符）；中文放 `title`。
- reindex 脚本只解析上述字段，多余字段会被忽略（但不报错）。
