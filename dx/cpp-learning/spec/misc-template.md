---
title: 杂项条目模板
brief: misc/cpp/ 杂项文件的标准骨架：tip / pitfall / snippet 三类
level: default
---
# 杂项条目模板

C++ 学习产生的杂项（小技巧、踩坑、代码片段）放项目 `misc/cpp/`，目录与命名规则见 `dx:tutorial-authoring/spec/directory-structure.md` 的「杂项目录」节。每条一个文件，按下列骨架写。

## tip（技巧）骨架

```md
# tip：<一句话标题>

适用：C++17 · <场景>
<正文：结论 + 最小示例 + 适用边界>
相关：<指向 knowledge/ 笔记或教程的链接>
```

## pitfall（踩坑）骨架

```md
# pitfall：<现象一句话>

环境：<编译器 / 版本 / 平台>
现象：<报错信息或错误行为>
原因：<根因>
解法：
1. <最小修复步骤>
相关：<同类主题链接>
```

## snippet（片段）骨架

```md
# snippet：<用途一句话>

```cpp
// 最小可运行代码，注明依赖
```

用法：<调用示例>
```

## 规则
1. 文件命名 `NN-type-主题`（英文 kebab-case），如 `01-tip-string-view-dangling.md`、`02-pitfall-vector-iterator-invalidate.md`。
2. 每条只讲一个点，篇幅 ≤ 100 行；无章节结构。
3. 条目成熟后的升级路径：整理进 knowledge 对应文件，或扩写为教程（走父 dx 的 A/B 模式）。
