---
title: 学习笔记模板
brief: knowledge 笔记文件的标准结构：结论先行、最小示例、坑与待深入
level: default
---
# 学习笔记模板

把某主题**已确认**的结论回写 knowledge/ 时，按此结构组织；文件名 kebab-case、一主题一文件。默认执行，偏离须说明理由。

## 文件骨架

```md
---
title: <主题>
brief: <一句话>
tags: [cpp, …]
---
# <主题>

## <子主题>
- <结论先行：第一句给可执行结论，再展开细节>
- <引用出处或注明「实验验证」>

```cpp
// 最小可运行示例，注明 C++17
```

## 坑
- <现象 → 根因 → 解法一句话；详细展开可移入 misc/cpp/>

## 待深入
- <没吃透的点，作为下一轮学习入口>
```

## 规则
1. 每条结论必须有出处（cppreference、官方文档或跑通的实验）；无出处的结论标注「未验证」。
2. 抽象概念（类关系、调用时序、对象布局）按 `dx:tutorial-authoring/spec/diagrams.md` 配 mermaid 图。
3. 代码块标注语言；排版硬规则以 `dx:tutorial-authoring/spec/layout.md` 为准。
4. 「待深入」条目应能回链到 `dx:cpp-learning/knowledge/roadmap.md` 的阶段规划。
