---
title: 操作语义
brief: create/modify/delete/reindex 四类操作的行为与安全策略
---
# 操作语义

| 操作 | 行为 | 安全策略 |
|---|---|---|
| create | 建文件夹 + DESC.md + knowledge/、reference/（+ 可选 spec/）→ reindex | id 重名、father 引用不存在时 reindex 报 error 拒绝写入（脚本不代建父级，需先修复 DESC 或先创建父级） |
| modify | 改 DESC / 增删子文件 / 改 tags / 改 father → reindex | 改 father 须指向已存在 id 且同步改写正文关系段；children 自动重算 |
| delete | 删整个文件夹 → reindex | **有子 dx 拒绝删除**，先处理子级 father；别名与索引条目随 DESC 自动消失 |
| reindex | 从 DESC frontmatter + 文件系统重建全部 INDEX.json | 校验：id=文件夹名、father 引用存在、无环、无重名、必填字段 |

索引漂移的兜底原则：文件系统与 DESC 是事实，INDEX 永远可重建；任何时候手改坏了直接重跑脚本。
