# Skill 审查报告（分主题规划）

> 审查对象：`.agents/skills/` 下 KB 三件套 —— kb-create / kb-add / kb-use
> 方式：5 个并行 subagent（3 个单技能深审 + 1 个交叉一致性 + 1 个规范参照），关键结论均经实测或代码核实。

## 总体结论

三件套**整体质量高**：frontmatter 合规、UTF-8/Windows 适配到位、create→add→use 全链路实测跑通、INDEX 由 add 全量重建（幂等、不丢内容）。发现 **2 个中危**、若干低危问题，无高危。规范层面与 pi 本地文档（docs/skills.md，实现 agentskills.io 规范）逐条比对通过。

---

## 主题一：触发与路由（description 质量）

| 问题 | 位置 | 建议 |
|---|---|---|
| description 仅含中文触发词 | 三个 SKILL.md frontmatter | 各补 1-2 组英文说法（create/init a KB、add knowledge/entry、query/search KB），description 是唯一路由信号，英文提问当前不触发 |
| kb-use L1 未给可复制的 INDEX 路径示例 | kb-use/SKILL.md:25 | 补一行 `docs/<type>/INDEX.md` 示例，降低 agent 走错路径的概率 |

## 主题二：安全与数据完整性

| 级别 | 问题 | 位置 | 建议 |
|---|---|---|---|
| 中 | 防覆盖闸门只查 DESC.md：DESC 缺失（误删/手工建库）时**不带 --force 也会静默重置三个 INDEX.md**（实测追加行被清空） | init_kb.py:100-104 | INDEX.md 存在且含非模板数据行时同样拒绝，提示 --force |
| 低 | 条目文件直接 write_text 覆盖，写入中断可能截断（INDEX 可重建，风险低） | kb_add.py:221 等 | 临时文件 + rename 原子写 |
| 低 | frontmatter 重复 key 静默后者覆盖；"标量 key 后跟列表项"的手工误编辑在 update 重写后静默丢数据且 validate 不报 | kb_add.py:56-65 | 解析时检测重复 key；该结构直接报错 |

## 主题三：查询行为一致性

| 级别 | 问题 | 位置 | 建议 |
|---|---|---|---|
| 中 | open 对跨类型同名 stem（tag/gil 与 spec/gil）静默返回第一个精确匹配，不像模糊匹配那样提示歧义 | kb_query.py:141-144 | 精确命中多个时也列候选、rc=1 退出 |
| 低 | search 无命中 exit 0、open 无匹配 exit 1，同为"未找到"退出码语义不一 | kb_query.py | 统一约定并在 SKILL.md 注明 |
| 低 | 带 BOM 的条目对 search/open/graph 静默不可见，但 locate 条数统计仍计入 | kb_query.py:23 | read_text 后 `lstrip("\ufeff")`，或跳过时打印提示 |
| 低 | terms 只匹配 "=" 左侧，右侧英文别名搜不到；docs() 已读全文，cmd_search 又二次 read_text（IO 翻倍） | kb_query.py:111, 63 | 右侧纳入匹配；docs() 一并返回 body |
| 低 | 坏 UTF-8 文件抛裸 traceback | kb_query.py:63 | docs() 加 try/except 友好报错 |

## 主题四：三件套闭环一致性

实测 create→add→validate→locate/search/open/graph→refresh-ref→validate **全链路跑通**；INDEX 模板 create 与 add 字节级等价。剩余契约缺口：

| 级别 | 问题 | 建议 |
|---|---|---|
| 中 | create 声明 tag 必备 scope/terms，但 add 缺省写占位值"（待补充）"、validate 只校验 id/type/title/created/updated —— "必备字段"契约未闭环 | validate 对 tag 增查 scope/terms，占位值视为缺失 |
| 低 | kb-use L1 称 INDEX 是检索入口，kb_query.py 实际绕过 INDEX 直扫 frontmatter，两套机制并存未声明优先级 | SKILL.md 注明"脚本结果为准，INDEX 过期提示 reindex" |
| 低 | CLI 入口风格不一：init_kb.py 位置参数 vs kb_add/kb_query 的 `--kb` | 统一为 `--kb` |
| 低 | refresh-ref 同分钟刷新显示 "23:24 -> 23:24"，无法佐证已更新 | 时间戳精度到秒，或输出 [OK] 即视为已更新 |

## 主题五：工程结构与可维护性

| 级别 | 问题 | 建议 |
|---|---|---|
| 低 | 脚本命名风格不一：init_kb.py（动词后置）vs kb_add.py / kb_query.py（kb 前缀） | 统一为 kb_init.py（需同步改 INDEX 模板注释里的引用） |
| 低 | `TYPE_LABELS` / INDEX 头注释在 init_kb.py 与 kb_add.py 各存一份，当前一致但有漂移风险 | 收敛到单一权威（公共模块或以 kb_add 为准、init 引用之） |
| 低 | init_kb.py 无 OSError/PermissionError 捕获，目标只读时裸 traceback；报错文案"路径不能以分隔符结尾"名不副实（实际只拦空/根路径） | 加捕获；文案改为"路径为空或为根目录" |

---

## 优先级行动计划

**P0（本周，中危 2 项）**
1. init_kb.py：INDEX 防覆盖闸门（主题二-中）
2. kb_query.py cmd_open：跨类型同名歧义提示（主题三-中）

**P1（下个迭代）**
3. validate 补齐 tag 必备字段校验，占位值视为缺失（主题四-中）
4. 三个 description 补英文触发词（主题一）
5. frontmatter 解析健壮性：重复 key、混合结构报错（主题二-低）

**P2（择机）**
6. 统一脚本命名 kb_init.py + 收敛 TYPE_LABELS 常量（主题五）
7. kb_query：BOM 处理、terms 右侧匹配、退出码统一、docs() 复用正文（主题三）
8. 原子写、异常捕获、refresh-ref 秒级时间戳（主题二/四/五 低危项）

---

## 附注

- **`.ver` 目录不存在**：工作区及用户目录均未找到，本次审查未使用其中的示例；补上后可按本报告口径复审。
- **网络不可用**：外网 5 个 URL 全部超时/SSL 失败。规范参照来自本机 pi 官方文档 docs/skills.md（实现 agentskills.io Agent Skills 规范，视为本地权威源）+ 模型内置知识（已标注，未经网络验证）。

---

## 修复记录（2024-11 本轮已全部落地，回归测试 25/25 通过）

| # | 修复项 | 落点 |
|---|---|---|
| 1 | INDEX 防覆盖闸门：INDEX.md 已有数据行时拒绝，提示 --force（中危） | kb_init.py `index_has_data` + main |
| 2 | open 跨类型同名 stem 精确命中多个时列候选并 exit 1，不再静默取第一个（中危） | kb_query.py `cmd_open` |
| 3 | validate 补齐 tag 必备字段契约：scope/terms 缺失或占位值“（待补充）”均报问题（中危） | kb_add.py `cmd_validate` |
| 4 | 三个 description 补英文触发词 | 三个 SKILL.md frontmatter |
| 5 | kb-use L1 补 INDEX 路径示例；注明“脚本结果为准、INDEX 过期提示 reindex”； L2 注明退出码约定 | kb-use/SKILL.md |
| 6 | frontmatter 解析严格化：重复 key、标量后又跟列表项→ ValueError；validate/reindex/update/show 均友好报错，reindex 跳过时打印提示 | kb_add.py `parse_fm/load_doc` 等 |
| 7 | 原子写：全部落盘改为临时文件 + rename | kb_add.py `write_atomic`、kb_init.py `write` |
| 8 | spec 引用口径统一：`resolve_doc` 支持完整 id 或跨类型唯一裸名，add 与 validate 共用 | kb_add.py `resolve_doc` |
| 9 | 脚本重命名 init_kb.py → kb_init.py；CLI 统一为 `--kb`；TYPE_LABELS/INDEX_HEADER 收敛到 kb_add.py 单一权威（kb_init 优先导入，带回退副本） | kb-create 目录 |
| 10 | 时间戳精度提到秒（refresh-ref 同分钟刷新可区分）；validate 新鲜度比对兼容两种精度 | kb_add.py/kb_init.py `now()`、`ts_re` |
| 11 | BOM 兼容：读取统一 utf-8-sig（BOM 条目对 search/open/graph 可见）；坏 UTF-8 文件友好跳过提示 | kb_query.py `docs`、kb_add.py `load_doc` |
| 12 | kb_query：terms 右侧英文别名纳入匹配（权重 2 “术语定义”）；docs() 一并返回正文消除二次读盘 | kb_query.py |
| 13 | update 重写保留正文末尾换行 | kb_add.py `dump_fm` |
| 14 | init：OSError/PermissionError 捕获；报错文案改为“路径为空或是根目录” | kb_init.py |

回归测试覆盖：create/--force/双闸门拒绝、空库 validate、add 三类、裸名 spec 关联、契约校验、重复 key（validate/reindex/update 三路径）、BOM 条目、同名 stem 歧义、术语定义命中、refresh-ref 秒级区分、尾换行保留、无 .tmp 残留、reindex 后索引新鲜度、locate/graph。
