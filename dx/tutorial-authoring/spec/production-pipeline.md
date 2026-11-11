---
title: 教程制作流程规范
brief: 七阶段流程的执行规则、阶段门禁与发布检查清单
level: default
---
# 教程制作流程规范

按 `dx:tutorial-authoring/knowledge/production-pipeline.md` 的七阶段执行：

1. 每篇教程开工前必须有选题卡（一句话主题 + 读者画像 + 预期收获），写进教程文件的 YAML frontmatter（字段格式见 `dx:tutorial-authoring/spec/layout.md`）；无选题卡不开工。生成教程前还必须先过 `dx:tutorial-authoring/spec/research-confirm.md` 的调研与用户确认门禁（required，不可跳过）。
2. 调研阶段的每条外部结论都必须有出处（URL）与核实标记，集中存入调研记录；调研范围与深度按 research-confirm 规范执行。
3. 大纲确认（含 research-confirm 的用户二次确认）后才进入写作；写作中确需调整结构时，先改大纲再继续写。
4. 教程中的每条命令、每段示例代码，发布前必须在干净环境跑通；跑不通的示例不得发布——标注环境前提或删除。
5. 发布前逐项过发布检查清单：
   - 标题如实反映内容
   - 版本/环境信息明确（软件名 + 版本号 + 系统 + 成文日期）
   - 外部引用均有出处链接
   - 排版符合 `dx:tutorial-authoring/spec/layout.md`
   - 截图/图片可正常显示且 alt 文本完整
6. 发布后跟踪反馈；因版本升级导致步骤失效时，及时更新内容或在文首标注失效说明。

裁剪或偏离本流程时，须在教程任务记录中说明理由。
