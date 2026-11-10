---
title: spec 分级语义
brief: required/default/optional 三档 level 的确切含义与偏离要求
level: required
---
# spec 分级语义

| level | 含义 | 偏离要求 |
|---|---|---|
| `required` | 任何情形必须遵守 | 不允许；违反视为错误 |
| `default` | 默认执行 | 允许，但必须给出理由并记录在相关 dx 或任务记录中 |
| `optional` | 参考性做法 | 按情形自由采用，无需说明 |

使用规则：
- spec 文件 frontmatter 的 `level` 缺省时视为 `default`。
- 同一 dx 内规则冲突时，以 level 更高（更强制）者为准。
