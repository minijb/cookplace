---
name: kb-use
description: 渐进式使用本地知识库（DESC.md + docs/ 结构）：先只读 DESC.md，需要时再读 INDEX.md 索引，最后精确定位并只加载需要的单个文档。当用户要在知识库里“查/用知识库、检索、找规范/spec、查术语、看看知识库里有什么”时使用。Use when the user wants to query or search the knowledge base, find a spec or term, or see what's in the KB.
---

# kb-use · 渐进式使用知识库

知识库 = 一个文件夹：`DESC.md` + `docs/{tag,reference,spec}/`（各含 `INDEX.md` 索引）。
本 skill 的核心纪律：**按层加载，禁止一次读入 docs 全部内容**。上下文是预算：每层只在
真正需要时进入下一层，且每次只读定位到的单个文件。

## 加载协议（Level 0 → 3，逐级、按需）

### L0 · 概览（会话开始时只做这一步）

只读 `DESC.md`（含头部的用途一句话、作用/范围/功能/目录结构）。用它回答：
这个库是干嘛的？用户的问题在不在范围内？大概该去哪个类型（tag/reference/spec）找？
```bash
python scripts/kb_query.py locate --kb <kb>    # 汇总 DESC 摘要 + 各类型条数，不读 docs 内容
```
若 DESC 判断已足够回答（如“这个库的范围是什么”），直接回答，**不要**进入 docs。

### L1 · 索引（确需具体知识时）

只读**对应类型**的 `docs/<type>/INDEX.md`（如 `docs/tag/INDEX.md`、`docs/spec/INDEX.md`；自动生成的索引表：文件/标题/关键词/更新/关联）。
用户要术语/知识点 → `tag`；要外部资料原文 → `reference`；要规范 → `spec`。只读需要的那一个 INDEX。
INDEX 与脚本结果不一致时**以脚本为准**（search/open 直读条目 frontmatter），INDEX 过期说明需要 `reindex`。

### L2 · 定点（从索引锁定目标）

用检索脚本精确匹配，再用 read 工具**只读命中的那一两个文件**：
```bash
python scripts/kb_query.py search --kb <kb> 关键词1 关键词2   # 多关键词为 AND；--top N 限定条数(默认8)；输出排序命中与理由
python scripts/kb_query.py open   --kb <kb> <id|文件名|标题片段> # 输出绝对路径，交给 read 工具
```
在 INDEX 中能直接看清的（如某 spec 的状态、某条目的更新时间），**不必**打开文件。
退出码约定：`search` 无命中 exit 0（空结果是正常响应）；`open` 未找到或命中多个歧义时 exit 1。

### L3 · 关联（仅在需要追溯关系时）

tag 会引用它使用的 spec（`specs:` 字段）与参考网页（`refs:`，含载入时间）。
需要理解知识间的关系、或需要顺藤摸瓜时再看关系图，然后回到 L2 只读目标文件：
```bash
python scripts/kb_query.py graph --kb <kb>   # tag→spec、tag→参考网页、reference→source、spec→status
```

## 使用规则

1. **逐级加载**：跳级（上来就读 docs/ 里的具体文件、或一口气读三个 INDEX）只有在用户
   明确点名某个文件时才允许。
2. **只读所需**：命中多个文件时，先按 search 给出的理由挑最相关的 1–2 个读，不够再补。
3. **给出处**：回答中引用知识时附文件相对路径（如 `docs/tag/python-gil.md`）与更新时间。
4. **新鲜度**：引用参考网页结论时注意“载入时间”；用户要求核实最新内容时，联网核对后
   用 kb-add skill 的 `refresh-ref` 更新载入时间，再引用。
5. **发现库问题**（INDEX 过期、条目缺字段）：提示用户运行 `kb_add.py validate/reindex`，
   不要在 kb-use 流程中顺手大改知识库。
