---
name: kb-create
description: 创建/初始化一个本地知识库（KB）骨架：DESC.md + docs/{tag,reference,spec}/INDEX.md。当用户说“新建知识库、初始化知识库、创建 KB”时使用。Use when the user asks to create, initialize, or set up a knowledge base (KB).
---

# kb-create · 创建知识库

知识库 = 一个文件夹，只含两样东西：`DESC.md`（库的说明书）与 `docs/`（内部文档）。
`docs/` 下固定三个子目录，各带一个 `INDEX.md` 索引：

```text
<kb-name>/
├── DESC.md                 # 作用 / 范围 / 功能 / 目录结构（简略但必要）
└── docs/
    ├── tag/                # 内部知识标签
    │   └── INDEX.md        # 自动生成的索引表（MOC，勿手改）
    ├── reference/          # 外部引用文档
    │   └── INDEX.md
    └── spec/               # 规范（格式不定）
        └── INDEX.md
```

三种基础数据结构（每个条目 = 子目录下的一个 md 文件，头部为受限 frontmatter）：

| 类型 | 必备字段 | 可选字段 |
|---|---|---|
| `tag` | time(created/updated)、范围(scope)、术语(terms) | 使用的 spec(specs，可无)、要求(requirements，可无)、参考网页(refs：一句话概括 + 载入时间，可被更新) |
| `reference` | 最后更新时间(updated) | source(出处/URL) |
| `spec` | 最后更新时间(updated) | status(draft/active/deprecated)；正文格式完全自由 |

## 执行流程

1. **与用户确认三项信息**（缺一就先问，不要猜）：
   - 知识库的**路径与名称**；
   - **一句话用途**（将写入 DESC.md 顶部）；
   - **范围**：收录什么、不收录什么（可作为 DESC 草稿，供用户修改）。
2. **运行初始化脚本**（在本 skill 目录下）：
   ```bash
   python scripts/kb_init.py --kb "<知识库路径>" --name "<名称>" --desc "<一句话用途>"
   ```
   已存在 `DESC.md`、或任一 `INDEX.md` 已有数据时会拒绝；确认要重建时才加 `--force`。
3. **补全 DESC.md**：脚本会生成模板；将用户给出的“作用/范围/收录/不收录”要点填入对应小节，保持简略——DESC 是给别人（或 agent）快速判断“这个库是干嘛的”，不是正文。
4. **向用户汇报**：展示生成的目录树，并提示后续用 `kb-add` skill 添加知识、用 `kb-use` skill 使用知识库。

## 规则

- 只创建骨架，**不要**顺手编造内容填充 docs/；空 INDEX 是正常的。
- DESC.md 中“功能”一节说明渐进式加载方式（模板已含），不要删。
- 目录名、文件名用英文/拼音 slug；中文放在标题与正文里。
