---
title: triplets：目标平台与 ABI
series: cmake-vcpkg
chapter: part-02/02
topic: 理解三元组如何决定依赖库的平台与链接形态，避免 CRT 混链
audience: 前端游戏开发工程师
outcome: 会给项目选对 triplet、知道混用 triplet 的报错长相与根源
environment: Windows 11；vcpkg 2026-10-29；MSVC 14.39
date: 2026-11-14
---

# triplets：目标平台与 ABI

triplet（三元组）= 架构-操作系统-库形态，它决定 vcpkg 把每个依赖**编译成什么 ABI**。同一份 `vcpkg.json`，换 triplet 就是换一整套安装目录（`vcpkg_installed/<triplet>/`）。

| triplet | 产物形态 | CRT | 什么时候用 |
|---|---|---|---|
| `x64-windows` | 动态库（.dll） | 动态（/MD） | **默认**，本教程主线 |
| `x64-windows-static` | 静态库 | 静态（/MT） | 想单文件分发 exe、免 DLL 依赖 |
| `x64-windows-static-md` | 静态库 | 动态（/MD） | 静态链第三方、但与动态 CRT 的自家代码共存 |
| `x64-mingw-dynamic` | 动态库 | GCC 运行时 | 用 MSYS2/MinGW 编译器时 |

## 你将学会什么

triplet 与 ABI 的对应关系、怎么给构建指定 triplet、CRT 混链错误的识别。

## 为什么 triplet 决定一切：CRT 一致性

MSVC 世界里每个二进制都链接 C 运行时库（CRT）。你的 exe 用 `/MD`（动态 CRT）而依赖库用 `/MT`（静态 CRT）编出来时，链接器报 **LNK2038（检测到「_ITERATOR_DEBUG_LEVEL」的不匹配项）** 或运行期诡异崩溃。triplet 就是 vcpkg 用来保证「所有依赖 + 你的代码」统一 ABI 的开关。

> ⚠️ 前端背景的典型坑：机器上有 MSYS2 的 gcc 就把 VSCode kit 切成 GCC——GCC 编译的你的代码 + MSVC 编译的 vcpkg 依赖 = 必炸。**kit（编译器）与 triplet 必须配套**：MSVC kit 配 `x64-windows*`，GCC kit 配 `x64-mingw-*`。

## 指定 triplet 的三种方式

```powershell
# 1) 命令行（单次）
vcpkg install --triplet x64-windows-static-md

# 2) 环境变量（持久默认）
$env:VCPKG_DEFAULT_TRIPLET = "x64-windows-static-md"

# 3) CMake 缓存变量（推荐，随 preset 固化进仓库）
# CMakePresets.json 的 cacheVariables 里加：
#   "VCPKG_TARGET_TRIPLET": "x64-windows-static-md"
```

工程主线用第 3 种：preset 即文档，队友 clone 下来零配置一致。

## debug / release 双份从哪来

`vcpkg_installed/x64-windows/` 下有 `debug/` 和根级两套——vcpkg 默认**同时**安装 Debug 与 Release 两形态的依赖，链接时由 CMake 的配置自动挑选（Debug 构建链 `fmtd.dll`，Release 链 `fmt.dll`）。这也是首次安装慢一截的原因之一。

## 自定义 triplet（选修）

`<triplet>.cmake` 文件覆盖编译选项（如关异常、换 SDK 版本），通过 `--overlay-triplets` 注入。官方文档：<https://learn.microsoft.com/en-us/vcpkg/concepts/triplets>。在交叉编译（arm64 嵌入式/移动端）时是必经之路，主线用不到。

## 下一步

[03 · 版本控制：baseline 与锁定](03-versioning.md)。
