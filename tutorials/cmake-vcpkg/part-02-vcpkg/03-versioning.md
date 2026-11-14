---
title: 版本控制：baseline 与锁定
series: cmake-vcpkg
chapter: part-02/03
topic: 用 builtin-baseline 锁定 ports tree 版本，用版本约束与 overrides 定死依赖
audience: 前端游戏开发工程师，类比 lockfile 语义
outcome: 团队任何人任何时间 clone 项目，装出的依赖版本完全一致
environment: Windows 11；vcpkg 2026-10-29；baseline 91f0baffae4e06a68a961d7f6493e8a1ca6775eb
date: 2026-11-14
---

# 版本控制：baseline 与锁定

npm 用 lockfile 锁版本；vcpkg 的锁由两层组成：

```mermaid
flowchart LR
  BL[builtin-baseline<br/>ports tree 的 git commit] -->|决定每个 port 的<br/>默认版本基线] DEPS[dependencies]
  DEPS -->|可选：版本约束 version>=] RES[解析出唯一版本集]
  OV[overrides<br/>强制指定版本] -.压过一切.-> Res2[最终版本集]
```

- **`builtin-baseline`**：一个 vcpkg 仓库的 commit hash——「ports tree 冻结在那一刻」。没有它，manifest 模式的版本解析直接报错
- **版本约束**（可选）：`"version>="` 指定下限
- **`overrides`**（可选）：无条件强制某版本，解决传递依赖打架

## 你将学会什么

写对 baseline、按需加约束、更新基线的标准动作。

## baseline 从哪来

就是 vcpkg 仓库的 git commit：

```powershell
PS> cd C:\dev\vcpkg
PS> git log -1 --format=%H
91f0baffae4e06a68a961d7f6493e8a1ca6775eb    # 把它写进 vcpkg.json
```

本教程所有示例工程用的都是这个 hash（2026-11-13 的 ports tree）。队友不需要 checkout 到这个 commit——vcpkg 会从本地 git 历史自动解析。

## 版本约束示例

```json
{
  "name": "my-game",
  "version-string": "0.1.0",
  "builtin-baseline": "91f0baffae4e06a68a961d7f6493e8a1ca6775eb",
  "dependencies": [
    { "name": "fmt", "version>=": "10.2.1" },
    { "name": "glfw3" }
  ],
  "overrides": [
    { "name": "fmt", "version": "11.1.4" }
  ]
}
```

语法速查（manifest 里的版本字段是 `version>=`，与 JSON 转义无关，就这么写）：

| 字段 | 语义 |
|---|---|
| `"version>=": "10.2.1"` | 解析时取 ≥ 此版本（在 baseline 范围内） |
| `"overrides": [{ "version": "11.1.4" }]` | 无条件定死——依赖树的任何角落都解析到这个版本 |

## 升级依赖的标准动作

```powershell
PS> cd C:\dev\vcpkg
PS> git pull                                      # ports tree 前进
PS> git log -1 --format=%H                        # 新 hash
PS> cd <项目根>
PS> vcpkg x-update-baseline --baseline-id <新hash> # 或手改 vcpkg.json
PS> vcpkg upgrade --no-dry-run                    # 按 manifest 重装过时项
```

`x-` 前缀是官方标注的实验性命令，`x-update-baseline` 目前是更新基线的唯一自动化入口。升级后删 `build/` 重 configure。

> ⚠️ 只 `git pull` 不改 baseline 等于什么都没发生——manifest 工程的版本由 `vcpkg.json` 说了算，vcpkg 安装目录的新版本不会自动进来。这是与 `npm update` 直觉相反的地方。

## 下一步

[04 · 缓存与 registries](04-caching-and-registries.md)。
