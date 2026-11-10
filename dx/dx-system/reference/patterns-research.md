---
title: 相关模式调研（2025-11）
brief: 渐进披露、分层记忆等外部方案的调研链接与要点；外部资料，未经内部核实
source: 多来源（见正文链接）
fetched: 2025-11-09
---
# 相关模式调研（2025-11）

> reference 性质：以下是外部资料的摘要与链接，结论未在本工作区内部验证。

- Claude Agent Skills 最佳实践（渐进披露三级加载、SKILL.md <500 行）：
  https://platform.claude.com/docs/en/agents-and-tools/agent-skills/best-practices
- MEMM 架构（L0 一行摘要 / L1 操作摘要 / L2 全文的分层记忆）：
  https://memm.dev/docs/architecture-and-operating-model/
- 实用第二大脑：LLM 操作的 markdown 知识图：
  https://www.lirantal.com/blog/practical-second-brain-llm-markdown-knowledge-graph
- AI Agent 长期记忆的工程实践（markdown + 目录即记忆系统）：
  https://www.treygoff.com/writing/agent-long-term-memory
- agent-memory LAYOUT（一个事实一个文件、路径即分类）：
  https://github.com/Lolaplex/agent-memory/blob/main/abi/LAYOUT.md

对 dx 设计的启发（已吸收进本体系）：
1. 索引只放一句话 brief，正文按需加载（渐进披露）。
2. 元数据单一事实来源（DESC frontmatter）+ 可重建的索引缓存，避免双写漂移。
3. 文件要短，一个主题一个文件。
