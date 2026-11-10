---
title: 目录结构
brief: dx 工作区的完整目录结构与各部分职责
---
# 目录结构

```
<项目根>/
├── .agents/skills/          # dx-create / dx-manage / dx-explore 三个 skill
│   ├── dx-create/templates/ # DESC 与 knowledge/reference/spec 文件模板
│   └── dx-manage/scripts/   # reindex.py 索引重建脚本
└── dx/                      # 所有 dx 扁平存放
    ├── INDEX.json           # 总索引：units（id/title/brief/tags/aliases/father/children/has/files）+ tags 索引 + lookup（id/标题/别名 → dx）
    └── <id>/                # 一个 dx = 一个文件夹，id 即文件夹名
        ├── DESC.md          # 元数据 frontmatter + 描述正文
        ├── knowledge/INDEX.json + *.md
        ├── reference/INDEX.json + *.md   # 可选存在，但通常创建
        └── spec/INDEX.json + *.md        # 可选
```

要点：
- 扁平布局：父子关系只用 DESC 的 `father` 字段表达，`children` 由脚本反推；不物理嵌套。
- 术语路由：DESC 的 `title` 与 `aliases` 由 reindex 汇总为 `INDEX.json` 的 `lookup`（用户说法 → dx）；领域黑话放各 dx 的 `knowledge/terms.md`（可选）。
- `INDEX.json` 全部是生成物，禁止手改。
- 子目录 `INDEX.json` 的 items 含 `file/title/brief`，reference 另含 `source/fetched`，spec 另含 `level`。
