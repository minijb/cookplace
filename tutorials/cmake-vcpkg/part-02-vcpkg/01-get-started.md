---
title: vcpkg 上手：manifest 模式
series: cmake-vcpkg
chapter: part-02/01
topic: 安装引导 vcpkg，用 vcpkg.json 声明依赖并自动安装
audience: 前端游戏开发工程师，以 npm 为类比锚点
outcome: 能在项目里用 manifest 模式装好第一个依赖，看懂 vcpkg_installed 目录结构
environment: Windows 11；vcpkg 2026-10-29（安装于 C:\dev\vcpkg，已验证）
date: 2026-11-14
---

# vcpkg 上手：manifest 模式

vcpkg 的核心抽象是 **port**：一份「如何从源码构建某库」的配方（≈ npm 的 package + install script 合体）。官方配方集合叫 ports tree，随 vcpkg 仓库一起分发。先建立类比表，再看两种使用模式。

| npm 世界 | vcpkg 世界 |
|---|---|
| `package.json` | `vcpkg.json`（manifest） |
| `node_modules/` | `vcpkg_installed/` |
| `npm install` | `vcpkg install`（或 CMake configure 自动触发） |
| lockfile | `builtin-baseline`（part-02/03） |
| npm registry | ports tree（git 仓库形态） |

## 你将学会什么

classic 与 manifest 两种模式的边界、manifest 的写法与安装流程、产物目录怎么读。

## 两种模式

| | classic 模式 | manifest 模式（本教程只用这种） |
|---|---|---|
| 声明位置 | 命令行 `vcpkg install fmt` | 项目根 `vcpkg.json` |
| 安装位置 | vcpkg 安装目录（全局共享） | 项目内 `vcpkg_installed/` |
| 依赖漂移 | 容易（谁装过什么没人知道） | 不可能（声明即事实） |
| 适用 | 临时实验 | 一切正式项目 |

## 最小 manifest

```json
{
  "name": "fmt-demo",
  "version-string": "0.1.0",
  "builtin-baseline": "91f0baffae4e06a68a961d7f6493e8a1ca6775eb",
  "dependencies": ["fmt"]
}
```

字段速查：`name`（小写连字符）、`version-string`（项目自己的版本）、`builtin-baseline`（锁定 ports tree 的 commit，详解见 part-02/03）、`dependencies`（port 名列表）。

加依赖不用手改 JSON（也行），用命令（实测行为）：

```powershell
PS> vcpkg add port fmt
Succeeded in adding ports to vcpkg.json file.
```

查 port 名与版本：<https://vcpkg.io>（搜到后名字即 port 名），或 `vcpkg search glfw`。

## 安装：命令行 or CMake 自动触发

```powershell
PS> cd <项目根>          # 有 vcpkg.json 的目录
PS> vcpkg install        # 无参数：读 manifest 装
```

更常用的方式是**什么都不做**——CMake configure 接上 vcpkg 工具链后自动执行（part-03/01 实测输出）。首次安装是从源码构建（本机实测 glfw3 + glad 共 36 秒），之后进二进制缓存，第二次起秒级（part-02/04）。

## 读懂 vcpkg_installed/

```text
vcpkg_installed/
└── x64-windows/                 # triplet 名（part-02/02）
    ├── debug/                   # Debug 专用：debug\bin\fmtd.dll
    ├── include/                 # 头文件：fmt/format.h、fmt/color.h
    ├── bin/    lib/             # Release 的 .dll / .lib
    └── share/                   # 每个 port 的 CMake 配置包：share/fmt/fmtConfig.cmake
```

`share/<port>/*Config.cmake` 就是 find_package（CONFIG 模式）的对接面——part-01/07 的伏笔在这里闭合。整个目录进 `.gitignore`（≈ `node_modules/`）。

> ⚠️ 依赖装在 `build/` 旁边还是项目根，取决于工具链配置；默认 manifest 安装发生在**构建目录**内（`build/vcpkg_installed/`），删 build 目录时一并删除，属正常行为。

## 下一步

[02 · triplets：目标平台与 ABI](02-triplets.md)。
