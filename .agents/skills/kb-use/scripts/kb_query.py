#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""kb_query.py — 知识库渐进式查询工具（配合 kb-use skill：L0 概览 → L1 索引 → L2 定点 → L3 关联）。

用法（--kb 指向知识库根目录）:
  locate   L0: DESC 摘要 + 各类型条数（不读 docs 内容）
  search   L2: 关键词检索（多词 AND），按相关度排序，输出命中理由
  open     L2: 把 id / 文件名 / 标题片段解析为绝对路径（交给 read 工具）
  graph    L3: 打印知识间关系（tag→spec、tag→参考网页、reference→source、spec→status）

索引文件 docs/<type>/INDEX.md 为脚本生成物（见 kb_add.py），本工具只读不写。
"""
import argparse
import re
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")

TYPES = ("tag", "reference", "spec")
FM_RE = re.compile(r"^---\s*\n(.*?)\n---\s*\n?(.*)$", re.S)


def parse_fm(text: str):
    m = FM_RE.match(text)
    if not m:
        return {}, text
    meta: dict = {}
    key = None
    for ln in m.group(1).splitlines():
        mi = re.match(r"^\s*-\s+(.*)$", ln)
        if mi and key:
            meta.setdefault(key, [])
            if isinstance(meta[key], list):
                meta[key].append(mi.group(1).strip())
            continue
        mk = re.match(r"^([A-Za-z][\w-]*):\s*(.*)$", ln)
        if mk:
            key = mk.group(1)
            val = mk.group(2).strip()
            meta[key] = val if val else []
    return meta, m.group(2)


def kb_root(kb: str) -> Path:
    root = Path(kb).expanduser()
    if not (root / "DESC.md").exists():
        sys.exit(f"[错误] {root} 不是知识库（缺少 DESC.md）")
    return root


def docs(root: Path):
    """迭代 (type, path, meta, body)。坏编码文件跳过并提示（不计入结果）。"""
    for t in TYPES:
        folder = root / "docs" / t
        if not folder.is_dir():
            continue
        for p in sorted(folder.glob("*.md")):
            if p.name == "INDEX.md":
                continue
            try:
                text = p.read_text(encoding="utf-8-sig")
            except UnicodeDecodeError:
                print(f"[提示] 跳过 {t}/{p.stem}：不是有效的 UTF-8 文件", file=sys.stderr)
                continue
            meta, body = parse_fm(text)
            if meta:
                yield t, p, meta, body


# ---------- 子命令 ----------

def cmd_locate(a):
    root = kb_root(a.kb)
    text = (root / "DESC.md").read_text(encoding="utf-8-sig")
    # DESC 摘要：标题行 + 引用行 + 各节前几行
    lines = [ln.rstrip() for ln in text.splitlines() if ln.strip()]
    keep, count = [], 0
    for ln in lines:
        if ln.startswith("#"):
            keep.append(ln)
            count = 0
            continue
        if count < 2 and not ln.startswith("```"):
            keep.append(ln)
            count += 1
    print(f"知识库: {root.resolve()}")
    print("—— DESC.md 摘要 ——")
    print("\n".join(keep[:20]))
    print("—— 条目统计 ——")
    for t in TYPES:
        folder = root / "docs" / t
        n = len([p for p in folder.glob("*.md") if p.name != "INDEX.md"]) if folder.is_dir() else 0
        print(f"  docs/{t}/: {n} 条")


def cmd_search(a):
    root = kb_root(a.kb)
    kws = [k.lower() for k in a.keywords]
    hits = []
    for t, p, meta, body in docs(root):
        title = str(meta.get("title", ""))
        scope = str(meta.get("scope", ""))
        terms = meta.get("terms") or []
        body_l = body.lower()
        score, reasons = 0, []
        for kw in kws:
            w = 0
            why = ""
            if kw in p.stem.lower():
                w, why = 5, "文件名"
            elif kw in title.lower():
                w, why = 4, "标题"
            elif any(kw in t_.split("=", 1)[0].lower() for t_ in terms):
                w, why = 3, "术语"
            elif any("=" in t_ and kw in t_.split("=", 1)[1].lower() for t_ in terms):
                w, why = 2, "术语定义"
            elif kw in scope.lower():
                w, why = 2, "范围"
            elif kw in body_l:
                w, why = 1, "正文"
            if w == 0:
                score = -1
                break
            score += w
            reasons.append(f"{why}↔{kw}")
        if score >= 0:
            hits.append((score, t, p, meta, reasons))
    if not hits:
        print("[无命中] 可换关键词，或先读对应 docs/<type>/INDEX.md 人工浏览（L1）")
        return
    hits.sort(key=lambda h: h[0], reverse=True)
    print(f"[命中 {len(hits)} 条，按相关度排序，请只读最相关的 1–2 条]")
    for score, t, p, meta, reasons in hits[: a.top]:
        specs = ",".join(meta.get("specs") or []) or "—"
        print(f"  {score:>2}  {t}/{p.stem}  《{meta.get('title', '')}》  更新:{meta.get('updated', '—')}  spec:{specs}")
        print(f"       理由: {'; '.join(reasons)}")


def cmd_open(a):
    root = kb_root(a.kb)
    q = a.query.strip().strip("/")
    exact, fuzzy = [], []
    for t, p, meta, _ in docs(root):
        did = f"{t}/{p.stem}"
        if q == did or q == p.name or q == p.stem:
            exact.append((did, meta.get("title", "")))
        elif q.lower() in did.lower() or q.lower() in str(meta.get("title", "")).lower():
            fuzzy.append((did, meta.get("title", "")))
    if len(exact) == 1:
        t, name = exact[0][0].split("/", 1)
        print((root / "docs" / t / f"{name}.md").resolve())
        return
    if exact:  # 跨类型同名 stem 等场景：精确命中多个也要消歧，不静默取第一个
        print(f"[精确匹配到 {len(exact)} 个条目，请用完整 id（类型/名称）区分]")
        for did, title in exact[:10]:
            print(f"  {did}  《{title}》")
        sys.exit(1)
    if len(fuzzy) == 1:
        t, name = fuzzy[0][0].split("/", 1)
        print((root / "docs" / t / f"{name}.md").resolve())
    elif fuzzy:
        print("[多个匹配，请用更精确的 id]")
        for did, title in fuzzy[:10]:
            print(f"  {did}  《{title}》")
        sys.exit(1)
    else:
        sys.exit(f"[未找到] {q}；可用 search 先检索")


def cmd_graph(a):
    root = kb_root(a.kb)
    print("—— 知识关系图 ——")
    for t, p, meta, _ in docs(root):
        node = f"{t}/{p.stem}"
        if t == "tag":
            for s in (meta.get("specs") or []):
                print(f"  {node} --使用--> {s}")
            for r in (meta.get("refs") or []):
                parts = [x.strip() for x in r.split("|")]
                url = parts[0] if parts else "?"
                loaded = parts[2] if len(parts) > 2 else "?"
                print(f"  {node} --参考--> {url}  (载入 {loaded})")
        elif t == "reference":
            print(f"  {node} [source] {meta.get('source', '—')}  (更新 {meta.get('updated', '—')})")
        else:
            print(f"  {node} [status] {meta.get('status', '—')}  (更新 {meta.get('updated', '—')})")
    print("提示：spec 是否被引用可反向 grep；ref 的载入时间可用 kb_add.py refresh-ref 更新")


def main():
    ap = argparse.ArgumentParser(description="知识库渐进式查询")
    sub = ap.add_subparsers(dest="cmd", required=True)
    for name, fn, help_ in (("locate", cmd_locate, "L0 概览"), ("graph", cmd_graph, "L3 关系图")):
        p = sub.add_parser(name, help=help_)
        p.add_argument("--kb", required=True)
        p.set_defaults(fn=fn)
    p = sub.add_parser("search", help="L2 关键词检索")
    p.add_argument("--kb", required=True)
    p.add_argument("--top", type=int, default=8)
    p.add_argument("keywords", nargs="+")
    p.set_defaults(fn=cmd_search)
    p = sub.add_parser("open", help="L2 解析路径")
    p.add_argument("--kb", required=True)
    p.add_argument("query")
    p.set_defaults(fn=cmd_open)
    a = ap.parse_args()
    a.fn(a)


if __name__ == "__main__":
    main()
