#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""kb_init.py — 创建基础知识库骨架：DESC.md + docs/{tag,reference,spec}/INDEX.md

用法:
  python kb_init.py --kb <kb路径> [--name 名称] [--desc 一句话用途] [--force]

INDEX 格式的权威实现是 kb-add skill 的 kb_add.py（INDEX 由其 reindex 维护），
本脚本优先复用其常量，保证两边格式永不漂移；kb_add.py 不可用时回退到本地副本。
"""
import argparse
import sys
from datetime import datetime
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")

# 回退副本：须与 kb-add/scripts/kb_add.py 中的定义保持一致
_FALLBACK_TYPE_LABELS = {
    "tag": "tag · 内部知识标签 · INDEX",
    "reference": "reference · 外部引用 · INDEX",
    "spec": "spec · 规范 · INDEX",
}
_FALLBACK_INDEX_HEADER = (
    "<!-- AUTO-INDEX: 由 kb 脚本生成，请勿手工编辑；用 `kb_add.py reindex` 重建 -->")


def _load_shared():
    src = Path(__file__).resolve().parents[2] / "kb-add" / "scripts" / "kb_add.py"
    import importlib.util
    spec = importlib.util.spec_from_file_location("kb_add_shared", src)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod.TYPE_LABELS, mod.INDEX_HEADER


try:
    TYPE_LABELS, INDEX_HEADER = _load_shared()
except Exception:
    TYPE_LABELS = dict(_FALLBACK_TYPE_LABELS)
    INDEX_HEADER = _FALLBACK_INDEX_HEADER

INDEX_TMPL = (
    INDEX_HEADER + "\n# {label}\n\n> 共 0 条 · 重建于 {ts}\n\n"
    "| 文件 | 标题 | 关键词 | 更新 | 关联 |\n|------|------|--------|------|------|\n")

DESC_TMPL = """# {name} · 知识库

> {desc}

## 作用

- {desc}
- （待补充：该库为谁解决什么问题）

## 范围

- 收录：（待补充，例：与 XX 相关的概念、规范、外部资料）
- 不收录：（待补充，例：与 XX 无关的内容、临时草稿）

## 功能

本库按**渐进式加载**使用：

1. 先读本文件（`DESC.md`）了解库的作用与范围；
2. 需要具体知识时，读 `docs/<类型>/INDEX.md` 索引；
3. 按索引精确定位，只读需要的单个文档。

- `docs/tag/` —— 内部知识标签：时间、范围、术语、使用的 spec（可无）、要求（可无）、参考网页（一句话概括 + 载入时间，可被更新）
- `docs/reference/` —— 外部引用文档，含最后更新时间
- `docs/spec/` —— 规范，含最后更新时间，正文格式不定

## 目录结构

```text
{name}/
├── DESC.md            # 本文件：作用 / 范围 / 功能 / 目录结构
└── docs/
    ├── tag/
    │   └── INDEX.md   # 内部知识标签索引
    ├── reference/
    │   └── INDEX.md   # 外部引用索引
    └── spec/
        └── INDEX.md   # 规范索引
```
"""


def now() -> str:
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")


def write(p: Path, content: str) -> None:
    """临时文件 + rename，避免写入中断截断文件。"""
    p.parent.mkdir(parents=True, exist_ok=True)
    tmp = p.with_name(p.name + ".tmp")
    tmp.write_text(content, encoding="utf-8", newline="\n")
    tmp.replace(p)


def index_has_data(p: Path) -> bool:
    """INDEX.md 是否已有数据行（区别于刚初始化的空模板）；不存在视为空。"""
    if not p.exists():
        return False
    try:
        return any(ln.startswith("| [")
                   for ln in p.read_text(encoding="utf-8-sig").splitlines())
    except OSError:
        return True  # 读不到时保守视为有数据，交给 --force 决定


def main() -> None:
    ap = argparse.ArgumentParser(description="初始化知识库骨架")
    ap.add_argument("--kb", required=True, help="知识库根目录（不存在则创建）")
    ap.add_argument("--name", help="知识库名称（默认取目录名）")
    ap.add_argument("--desc", default="（待补充：一句话说明该库的用途）",
                    help="一句话用途，写入 DESC.md 顶部")
    ap.add_argument("--force", action="store_true",
                    help="DESC.md 已存在或 INDEX.md 已有数据时仍覆盖")
    a = ap.parse_args()

    root = Path(a.kb).expanduser()
    if not root.name:
        sys.exit("[错误] 路径为空或是根目录")
    name = a.name or root.name
    root.mkdir(parents=True, exist_ok=True)

    desc = root / "DESC.md"
    if not a.force:
        if desc.exists():
            sys.exit(f"[跳过] {desc} 已存在（知识库可能已初始化）。确认重建请加 --force。")
        dirty = [t for t in TYPE_LABELS
                 if index_has_data(root / "docs" / t / "INDEX.md")]
        if dirty:
            sys.exit(f"[跳过] docs/{{{','.join(dirty)}}}/INDEX.md 已有数据"
                     "（知识库可能已初始化）。确认重建请加 --force。")

    write(desc, DESC_TMPL.format(name=name, desc=a.desc))
    for sub, label in TYPE_LABELS.items():
        write(root / "docs" / sub / "INDEX.md",
              INDEX_TMPL.format(label=label, ts=now()))

    print(f"[OK] 知识库已创建：{root.resolve()}")
    for p in sorted(root.rglob("*")):
        if p.is_file():
            print("   ", p.relative_to(root).as_posix())
    print("\n下一步：")
    print("  1. 补全 DESC.md 的「作用 / 范围」（保持简略）")
    print("  2. 用 kb-add skill 向 docs/ 添加知识（tag / reference / spec）")


if __name__ == "__main__":
    try:
        main()
    except OSError as e:
        sys.exit(f"[错误] {e}")
