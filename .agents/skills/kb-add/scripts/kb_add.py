#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""kb_add.py — 向知识库 docs/ 添加/更新知识（tag / reference / spec），自动维护 INDEX.md。

知识库结构：DESC.md + docs/{tag,reference,spec}/INDEX.md（见 kb-create skill）。

用法（--kb 指向知识库根目录）:
  add          添加一条知识
  update       手工编辑文件后：刷新 updated 时间戳并重建该类型索引
  refresh-ref  更新 tag 中某参考网页的载入时间（按 url 匹配，无则追加）
  reindex      依据 frontmatter 重建全部 INDEX.md
  validate     校验结构、必填字段、引用与索引新鲜度
  show         打印某条知识的元数据

条目文件 = docs/<type>/<slug>.md，头部受限 frontmatter：
  标量 `key: value`；列表 `key:` 后跟若干 `  - item`。
  terms 项格式 "名称 = 定义"；refs 项格式 "url | 一句话概括 | 载入时间"。
"""
import argparse
import re
import sys
import unicodedata
from datetime import datetime
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")

TYPES = ("tag", "reference", "spec")
TYPE_LABELS = {
    "tag": "tag · 内部知识标签 · INDEX",
    "reference": "reference · 外部引用 · INDEX",
    "spec": "spec · 规范 · INDEX",
}
INDEX_HEADER = "<!-- AUTO-INDEX: 由 kb 脚本生成，请勿手工编辑；用 `kb_add.py reindex` 重建 -->"


def now() -> str:
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")


# ---------- 受限 frontmatter 解析/生成 ----------

def parse_fm(text: str):
    """返回 (meta, body)。格式非法（重复 key、标量后又跟列表项）时抛 ValueError。"""
    lines = text.splitlines()
    if not lines or lines[0].strip() != "---":
        return {}, text
    meta: dict = {}
    key = None
    seen: set = set()
    for i, ln in enumerate(lines[1:], start=1):
        if ln.strip() == "---":
            return meta, "\n".join(lines[i + 1:])
        m_item = re.match(r"^\s*-\s+(.*)$", ln)
        if m_item and key:
            if isinstance(meta.get(key), str):
                raise ValueError(f"frontmatter 字段 {key} 先为标量后又跟列表项")
            meta.setdefault(key, [])
            meta[key].append(m_item.group(1).strip())
            continue
        m_kv = re.match(r"^([A-Za-z][\w-]*):\s*(.*)$", ln)
        if m_kv:
            key = m_kv.group(1)
            if key in seen:
                raise ValueError(f"frontmatter 重复字段 {key}")
            seen.add(key)
            val = m_kv.group(2).strip()
            meta[key] = val if val else []
    return {}, text  # 没有闭合的 ---，视为无 frontmatter


def dump_fm(meta: dict, body: str) -> str:
    out = ["---"]
    for k, v in meta.items():
        if isinstance(v, list):
            out.append(f"{k}:")
            out.extend(f"  - {it}" for it in v)
        else:
            out.append(f"{k}: {v}")
    out.append("---")
    text = "\n".join(out) + "\n\n" + body.lstrip("\n")
    return text if text.endswith("\n") else text + "\n"


def load_doc(path: Path):
    """读取条目并解析 frontmatter；格式非法时抛 ValueError（消息含文件名）。"""
    try:
        meta, body = parse_fm(path.read_text(encoding="utf-8-sig"))
    except ValueError as e:
        raise ValueError(f"{path.name}: {e}") from None
    return meta, body


def write_atomic(p: Path, content: str) -> None:
    """临时文件 + rename，避免写入中断截断既有文件。"""
    p.parent.mkdir(parents=True, exist_ok=True)
    tmp = p.with_name(p.name + ".tmp")
    tmp.write_text(content, encoding="utf-8", newline="\n")
    tmp.replace(p)


def resolve_doc(root: Path, doc_id: str) -> Path | None:
    """把条目 id 解析为路径：支持完整 id（如 spec/xxx）或裸名（跨类型唯一时）。
    找不到/歧义时返回 None。add 与 validate 共用此口径。"""
    doc_id = doc_id.strip().strip("/")
    if "/" in doc_id:
        t, name = doc_id.split("/", 1)
        if t not in TYPES:
            return None
        p = root / "docs" / t / f"{name}.md"
        return p if p.exists() else None
    matches = [p for t in TYPES for p in (root / "docs" / t).glob(f"{doc_id}.md")]
    return matches[0] if len(matches) == 1 else None


# ---------- 路径与 slug ----------

def kb_root(kb: str) -> Path:
    root = Path(kb).expanduser()
    if not (root / "DESC.md").exists():
        sys.exit(f"[错误] {root} 不是知识库（缺少 DESC.md）")
    return root


def doc_path(root: Path, doc_id: str) -> Path:
    doc_id = doc_id.strip().strip("/")
    if "/" not in doc_id:
        matches = [p for t in TYPES for p in (root / "docs" / t).glob(f"{doc_id}.md")]
        if len(matches) == 1:
            return matches[0]
        sys.exit(f"[错误] 请用完整 id（如 tag/xxx）指定条目，找到 {len(matches)} 个匹配")
    t, name = doc_id.split("/", 1)
    if t not in TYPES:
        sys.exit(f"[错误] 类型必须是 {TYPES} 之一，得到 {t}")
    return root / "docs" / t / f"{name}.md"


def slugify(title: str) -> str:
    s = unicodedata.normalize("NFKD", title).encode("ascii", "ignore").decode()
    s = re.sub(r"[^A-Za-z0-9]+", "-", s).strip("-").lower()
    return s


def choose_slug(title: str, slug: str | None, type_: str) -> str:
    if slug:
        return slugify(slug) or slug
    s = slugify(title)
    if s:
        return s
    ts = datetime.now().strftime("%Y%m%d-%H%M%S")
    print(f"[提示] 标题无 ASCII 字符，自动命名 {type_}-{ts}（可用 --slug 指定）")
    return f"{type_}-{ts}"


# ---------- INDEX 生成 ----------

def keywords_of(type_: str, meta: dict) -> str:
    kws: list[str] = []
    if isinstance(meta.get("scope"), str) and meta["scope"]:
        kws.append(meta["scope"])
    for t in (meta.get("terms") or []):
        name = t.split("=", 1)[0].strip()
        if name:
            kws.append(name)
    if isinstance(meta.get("title"), str):
        kws.append(meta["title"])
    return "；".join(dict.fromkeys(kws)) or "—"


def related_of(type_: str, meta: dict) -> str:
    if type_ == "tag":
        specs = meta.get("specs") or []
        nrefs = len(meta.get("refs") or [])
        parts = []
        if specs:
            parts.append(",".join(specs))
        if nrefs:
            parts.append(f"refs×{nrefs}")
        return " ".join(parts) or "—"
    if type_ == "reference":
        return str(meta.get("source") or "—")
    return str(meta.get("status") or "—")


def cell(s: str) -> str:
    return s.replace("|", "／").replace("\n", " ").strip() or "—"


def build_index(root: Path, type_: str, quiet: bool = True) -> str:
    folder = root / "docs" / type_
    rows = []
    for p in sorted(folder.glob("*.md")):
        if p.name == "INDEX.md":
            continue
        try:
            meta, _ = load_doc(p)
        except ValueError as e:
            if not quiet:
                print(f"[提示] 跳过 {e}（未纳入索引，可运行 validate 体检）")
            continue
        if not meta:
            if not quiet:
                print(f"[提示] 跳过 {p.name}：缺少 frontmatter（未纳入索引）")
            continue
        rows.append((str(meta.get("updated", "")), p, meta))
    rows.sort(key=lambda r: r[0], reverse=True)

    lines = [INDEX_HEADER, f"# {TYPE_LABELS[type_]}", "",
             f"> 共 {len(rows)} 条 · 重建于 {now()}", "",
             "| 文件 | 标题 | 关键词 | 更新 | 关联 |",
             "|------|------|--------|------|------|"]
    for _, p, meta in rows:
        lines.append(
            f"| [{p.name}]({p.name}) | {cell(str(meta.get('title', p.stem)))} "
            f"| {cell(keywords_of(type_, meta))} | {cell(str(meta.get('updated', '—')))} "
            f"| {cell(related_of(type_, meta))} |")
    return "\n".join(lines) + "\n"


def reindex(root: Path, only: str | None = None) -> None:
    for t in ([only] if only else TYPES):
        p = root / "docs" / t / "INDEX.md"
        p.parent.mkdir(parents=True, exist_ok=True)
        write_atomic(p, build_index(root, t, quiet=False))
        print(f"[OK] 重建 {p.relative_to(root).as_posix()}")


# ---------- 子命令 ----------

def cmd_add(a):
    root = kb_root(a.kb)
    if a.type not in TYPES:
        sys.exit(f"[错误] --type 必须是 {TYPES} 之一")
    slug = choose_slug(a.title, a.slug, a.type)
    path = root / "docs" / a.type / f"{slug}.md"
    if path.exists():
        sys.exit(f"[错误] 已存在 {path}（内容有变请先手工编辑，再运行 update；或换 --slug）")

    meta: dict = {"id": f"{a.type}/{slug}", "type": a.type, "title": a.title,
                  "created": now(), "updated": now()}
    if a.type == "tag":
        meta["scope"] = a.scope or "（待补充）"
        meta["terms"] = a.term or []
        meta["specs"] = a.spec or []
        meta["requirements"] = a.req or []
        meta["refs"] = [normalize_ref(r) for r in (a.ref or [])]
    elif a.type == "reference":
        meta["source"] = a.source or "（待补充）"
    else:  # spec
        meta["status"] = a.status

    body = a.body or ""
    if a.body_file:
        body = Path(a.body_file).read_text(encoding="utf-8")
    if not body.strip():
        body = f"# {a.title}\n\n（待补充正文）\n"

    write_atomic(path, dump_fm(meta, body))
    reindex(root, a.type)
    print(f"[OK] 新增 {meta['id']} -> {path}")
    if a.type == "tag":
        missing = [s for s in (a.spec or []) if resolve_doc(root, s) is None]
        if missing:
            print(f"[提示] 关联的 spec 尚不存在：{', '.join(missing)}（可稍后用 add --type spec 创建）")


def normalize_ref(ref: str) -> str:
    """'url|概括' -> 'url|概括|now'；已是三段则原样返回。"""
    parts = [x.strip() for x in ref.split("|")]
    if len(parts) >= 3:
        return " | ".join(parts[:2] + [parts[2]])
    if len(parts) == 2:
        return f"{parts[0]} | {parts[1]} | {now()}"
    sys.exit(f'[错误] --ref 需要 "url|一句话概括" 格式，得到：{ref}')


def cmd_update(a):
    root = kb_root(a.kb)
    path = doc_path(root, a.id)
    try:
        meta, body = load_doc(path)
    except ValueError as e:
        sys.exit(f"[错误] {e}")
    if not meta:
        sys.exit(f"[错误] {path} 没有 frontmatter")
    meta["updated"] = now()
    write_atomic(path, dump_fm(meta, body))
    reindex(root, meta.get("type", path.parent.name))
    print(f"[OK] 已刷新 {a.id} 的 updated -> {meta['updated']}")


def cmd_refresh_ref(a):
    root = kb_root(a.kb)
    path = doc_path(root, a.id)
    try:
        meta, body = load_doc(path)
    except ValueError as e:
        sys.exit(f"[错误] {e}")
    if meta.get("type") != "tag":
        sys.exit("[错误] refresh-ref 只适用于 tag 条目")
    url = a.ref.split("|", 1)[0].strip()
    refs = meta.get("refs") or []
    new = normalize_ref(a.ref)
    for i, r in enumerate(refs):
        if r.split("|", 1)[0].strip() == url:
            old_loaded = r.split("|")[-1].strip() if "|" in r else "—"
            refs[i] = new
            meta["updated"] = now()
            write_atomic(path, dump_fm(meta, body))
            reindex(root, "tag")
            print(f"[OK] 更新载入时间：{url}\n     {old_loaded} -> {new.split('|')[-1].strip()}")
            return
    refs.append(new)
    meta["refs"] = refs
    meta["updated"] = now()
    write_atomic(path, dump_fm(meta, body))
    reindex(root, "tag")
    print(f"[OK] 追加参考网页：{url}")


def cmd_validate(a):
    root = kb_root(a.kb)
    problems = []
    titles: dict[str, list[str]] = {}
    for t in TYPES:
        folder = root / "docs" / t
        if not folder.is_dir():
            problems.append(f"缺少目录 docs/{t}/")
            continue
        if not (folder / "INDEX.md").exists():
            problems.append(f"缺少 docs/{t}/INDEX.md（可运行 reindex 生成）")
        for p in folder.glob("*.md"):
            if p.name == "INDEX.md":
                continue
            doc_id = f"{t}/{p.stem}"
            try:
                meta, _ = load_doc(p)
            except ValueError as e:
                problems.append(f"{doc_id}: {e}")
                continue
            if not meta:
                problems.append(f"{doc_id}: 缺少/无法解析 frontmatter")
                continue
            for field in ("id", "type", "title", "created", "updated"):
                if not meta.get(field):
                    problems.append(f"{doc_id}: 缺少必填字段 {field}")
            if meta.get("id") != doc_id:
                problems.append(f"{doc_id}: id 应为 {doc_id}，实际 {meta.get('id')}")
            if t == "tag":
                if not isinstance(meta.get("scope"), str) or meta.get("scope") in ("", "（待补充）"):
                    problems.append(f"{doc_id}: tag 缺少必填字段 scope（“（待补充）”视为缺失）")
                if not (meta.get("terms") or []):
                    problems.append(f"{doc_id}: tag 缺少必填字段 terms（至少一条 “名称 = 定义”）")
            titles.setdefault(str(meta.get("title")), []).append(doc_id)
            for r in (meta.get("refs") or []):
                if len([x for x in r.split("|") if x.strip()]) < 2:
                    problems.append(f"{doc_id}: ref 需要 'url | 一句话概括 | 载入时间'，得到：{r}")
            for s in (meta.get("specs") or []):
                if resolve_doc(root, s) is None:
                    problems.append(f"{doc_id}: 关联的 spec 不存在：{s}")
    for title, ids in titles.items():
        if len(ids) > 1:
            problems.append(f"标题重复 “{title}”：{', '.join(ids)}")
    # INDEX 新鲜度（比对时剥离时间戳行，避免跨分钟后误报）
    ts_re = re.compile(r"重建于 \d{4}-\d{2}-\d{2} \d{2}:\d{2}(?::\d{2})?")

    def norm(s: str) -> str:
        return ts_re.sub("重建于 <ts>", s)

    for t in TYPES:
        idx = root / "docs" / t / "INDEX.md"
        if idx.exists() and norm(idx.read_text(encoding="utf-8-sig")) != norm(build_index(root, t)):
            problems.append(f"docs/{t}/INDEX.md 与实际内容不一致（运行 reindex 修复）")
    if problems:
        print(f"[发现问题 {len(problems)} 处]")
        for x in problems:
            print("  -", x)
        sys.exit(1)
    print("[OK] 校验通过：结构、必填字段、引用与索引均正常")


def cmd_show(a):
    root = kb_root(a.kb)
    path = doc_path(root, a.id)
    try:
        meta, _ = load_doc(path)
    except ValueError as e:
        sys.exit(f"[错误] {e}")
    if not meta:
        sys.exit(f"[错误] {path} 没有 frontmatter")
    print(f"文件: {path}")
    for k, v in meta.items():
        if isinstance(v, list):
            print(f"{k}:")
            for it in v:
                print(f"  - {it}")
        else:
            print(f"{k}: {v}")


def main():
    ap = argparse.ArgumentParser(description="知识库知识添加/维护工具")
    sub = ap.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("add", help="添加一条 tag/reference/spec")
    p.add_argument("--kb", required=True)
    p.add_argument("--type", required=True, choices=TYPES)
    p.add_argument("--title", required=True)
    p.add_argument("--slug")
    p.add_argument("--scope", help="tag: 范围")
    p.add_argument("--term", action="append", default=[], help='tag: "名称 = 定义"，可多次')
    p.add_argument("--spec", action="append", default=[], help="tag: 使用的 spec id，可多次")
    p.add_argument("--req", action="append", default=[], help="tag: 要求，可多次")
    p.add_argument("--ref", action="append", default=[], help='tag: "url|一句话概括"，可多次')
    p.add_argument("--source", help="reference: 出处/URL")
    p.add_argument("--status", default="active", choices=["draft", "active", "deprecated"], help="spec: 状态")
    p.add_argument("--body", help="正文（简单单行）")
    p.add_argument("--body-file", help="正文文件（多行内容推荐）")
    p.set_defaults(fn=cmd_add)

    p = sub.add_parser("update", help="刷新某条目的 updated 时间并重建索引")
    p.add_argument("--kb", required=True)
    p.add_argument("id", help="条目 id，如 tag/xxx")
    p.set_defaults(fn=cmd_update)

    p = sub.add_parser("refresh-ref", help="更新 tag 中参考网页的载入时间（按 url 匹配）")
    p.add_argument("--kb", required=True)
    p.add_argument("id")
    p.add_argument("--ref", required=True, help='"url|一句话概括"')
    p.set_defaults(fn=cmd_refresh_ref)

    p = sub.add_parser("reindex", help="重建全部 INDEX.md")
    p.add_argument("--kb", required=True)
    p.set_defaults(fn=lambda a: reindex(kb_root(a.kb)))

    p = sub.add_parser("validate", help="校验知识库")
    p.add_argument("--kb", required=True)
    p.set_defaults(fn=cmd_validate)

    p = sub.add_parser("show", help="查看某条目的元数据")
    p.add_argument("--kb", required=True)
    p.add_argument("id")
    p.set_defaults(fn=cmd_show)

    a = ap.parse_args()
    a.fn(a)


if __name__ == "__main__":
    main()
