---
name: kb-add
description: 向已有知识库（DESC.md + docs/ 结构）添加知识：新建/更新 tag、reference、spec，并自动维护 INDEX.md 索引。必须先与用户讨论对齐、确认后才落盘。当用户说“往知识库里添加/记录/沉淀知识、加一条 spec/reference/tag、更新参考网页载入时间”时使用。Use when the user asks to add or record knowledge into a KB, create a spec/reference/tag entry, or refresh a reference page's loaded-at time.
---

# kb-add · 向知识库添加知识

知识库结构由 `kb-create` skill 定义：`DESC.md + docs/{tag,reference,spec}/INDEX.md`。
本 skill 的核心纪律：**先对齐，后落盘**——没有和用户确认过的内容不许写入知识库。

## 标准流程（三阶段，严格执行）

### 阶段 1 · 对齐（必须）

1. 找到知识库根目录（含 `DESC.md` 的文件夹）；读 `DESC.md` 确认要加的内容**在范围内**。
   超范围 → 先向用户说明，由用户决定是否扩大范围（改 DESC）。
2. 判断条目类型，向用户呈现**待写入草稿**并确认：
   - **tag**（内部知识标签）：标题、范围、术语（名称=定义）、使用的 spec（可无）、要求（可无）、
     参考网页（每个 = 一句话概括 + 载入时间，之后可用 `refresh-ref` 更新载入时间）；
   - **reference**（外部引用文档）：标题、出处(URL)、要点/摘录正文；
   - **spec**（规范）：标题、状态(draft/active/deprecated)、规范正文（格式自由：表格/代码/清单均可）。
3. 草稿要具体到字段值（术语给定义、网页给一句话概括），用户说“可以/确认”才进入阶段 2。
   用户提供的是原始材料时，先整理成草稿再确认，不要原文照搬。

### 阶段 2 · 落盘（脚本写入，不要手写文件）

所有命令在本 skill 目录下运行；`--kb` 指向知识库根目录。
多行正文先写到临时文件再传 `--body-file`（简单单行正文可直接 `--body`），避免转义问题。

```bash
# 添加 tag（--term 可多个；--spec 关联规范 id，可多个；--req 要求，可多个；
# --ref 格式 "url|一句话概括"（载入时间自动取当前时间），可多个）
python scripts/kb_add.py add --kb <kb> --type tag --title "Python GIL" \
  --scope "Python 并发" \
  --term "GIL = 全局解释器锁，CPython 中……" \
  --spec "spec/cpython-internals" \
  --ref "https://wiki.python.org/moin/GlobalInterpreterLock|官方 wiki 对 GIL 的定义"

# 添加 reference（外部引用，正文放摘录/要点并注明出处）
python scripts/kb_add.py add --kb <kb> --type reference --title "PEP 703 摘要" \
  --source "https://peps.python.org/pep-0703/" --body-file note.md

# 添加 spec（正文格式自由）
python scripts/kb_add.py add --kb <kb> --type spec --title "提交信息规范" \
  --status active --body-file spec.md

# 其他维护命令
python scripts/kb_add.py update      --kb <kb> tag/python-gil      # 手工编辑后刷新 updated 时间并重建索引
python scripts/kb_add.py refresh-ref --kb <kb> tag/python-gil --ref "https://…|重新核对的概括"  # 更新参考网页载入时间
python scripts/kb_add.py reindex     --kb <kb>                     # 手工改动后重建全部 INDEX
python scripts/kb_add.py validate    --kb <kb>                     # 体检
python scripts/kb_add.py show        --kb <kb> tag/python-gil      # 查看某条元数据
```

文件名：默认由英文标题生成 slug；中文标题需显式给 `--slug`（英文/拼音），
否则脚本自动用 `tag-20260101-120000` 式名字。同 id 已存在会拒绝，改内容请走 `update`。

### 阶段 3 · 汇报

落盘后向用户报告：新增/变更文件的路径、条目 id、INDEX.md 的变化（脚本输出已含），
以及新条目与已有条目的关联（如 tag→spec）。如有超范围或疑似重复（validate 会查重名），主动提示。

## 条目文件格式（受限 frontmatter，脚本负责，人工编辑时也须遵守）

```markdown
---
id: tag/python-gil
type: tag
title: Python GIL
created: 2026-01-01 12:00:00
updated: 2026-01-01 12:00:00
scope: Python 并发
terms:
  - GIL = 全局解释器锁，……
specs:
  - spec/cpython-internals
requirements:
  - ……
refs:
  - https://… | 官方 wiki 对 GIL 的定义 | 2026-01-01 12:00:00
---
# Python GIL
（可选正文）
```

约定：`terms` 用 `名称 = 定义`；`refs` 用 `url | 一句话概括 | 载入时间`；
reference/spec 至少有 `updated`（最后更新时间）；spec 正文格式不定。
`INDEX.md` 全部由脚本生成，**禁止手工编辑**。

`validate` 会校验：必填字段（tag 的 `scope`/`terms` 缺失或为占位值“（待补充）”均视为问题）、
frontmatter 格式（重复 key、标量后又跟列表项会报错）、spec 引用（支持 `spec/xxx` 完整 id 或跨类型唯一的裸名）与索引新鲜度。
写入均为临时文件 + rename 原子落盘；时间戳精确到秒。
