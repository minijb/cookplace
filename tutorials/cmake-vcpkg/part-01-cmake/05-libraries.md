---
title: 库的类型：STATIC、SHARED 与 header-only
series: cmake-vcpkg
chapter: part-01/05
topic: 认识四类库目标，掌握 header-only 库的 INTERFACE 惯用法
audience: 前端游戏开发工程师，类比过 npm 包的 ESM/CJS/类型包
outcome: 会按场景选库类型，能用 INTERFACE 目标正确发布一个 header-only 库
environment: Windows 11；CMake 3.29.2；示例 code/part-01-05-libraries（已验证）
date: 2026-11-14
---

# 库的类型：STATIC、SHARED 与 header-only

`add_library` 有四种形态，先看全貌再逐个展开：

| 类型 | Windows 产物 | 链接期行为 | 运行期依赖 |
|---|---|---|---|
| `STATIC` | `.lib`（实现全在里面） | 复制进可执行文件 | 无 |
| `SHARED` | `.dll` + 导入 `.lib` | 只复制引用 | 运行时需找到 `.dll` |
| `OBJECT` | `.obj` 集合 | 不出库，被下游直接吸收 | 无 |
| `INTERFACE` | 无产物 | 只传播用法要求 | 无 |

类比 npm：STATIC ≈ 打进 bundle 的依赖；SHARED ≈ external/peer dependency（运行时要有实体）；INTERFACE ≈ `@types/*` 纯声明包。

## 你将学会什么

四种库的取舍、header-only 库的 INTERFACE 惯用法、Windows 下 DLL 的部署问题（part-03 会被 vcpkg 自动解决）。

## header-only 库：INTERFACE 目标

示例代码：[code/part-01-05-libraries](../code/part-01-05-libraries/)。INTERFACE 目标没有编译产物，只描述「消费我需要什么」：

```cmake
add_library(veclib INTERFACE)
target_include_directories(veclib INTERFACE include)
# 语义要求也传播：任何链接 veclib 的目标自动获得 C++17
target_compile_features(veclib INTERFACE cxx_std_17)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE veclib)
```

注意这里的 `INTERFACE include`：veclib 自己没有源文件要编译，一切用法要求只为下游而设。构建运行（本机验证）：

```powershell
PS> cmake --build build
PS> .\build\app.exe
len = 5.0
```

## STATIC vs SHARED 怎么选

- **默认 STATIC**：游戏与图形工程几乎全线静态链接——少一个运行时 DLL 查找问题，启动快，符号裁剪好做
- SHARED 的场景：库极大且被多个进程共享（引擎编辑器 + 游戏）、需要热替换、许可证要求动态隔离
- Windows 的 SHARED 有额外成本：导出符号表（`WINDOWS_EXPORT_ALL_SYMBOLS` 或显式 `__declspec(dllexport)`）、部署期把 `.dll` 放到 `.exe` 旁

> ⚠️ vcpkg 的 `x64-windows` triplet 装出来的第三方库是 DLL 形态——依赖的 DLL 必须与 exe 同目录，否则运行时报「找不到 dll」。vcpkg 工具链会自动复制（part-03/01 实测），自己手搭 SHARED 时要自己处理。

## OBJECT 库一句话

`OBJECT` 是「一堆 .obj 的打包」：多个可执行文件想共享编译产物但不想出 .lib 时用。日常用得少，见到不陌生即可。

## 下一步

[06 · 变量与缓存](06-variables-and-cache.md)——configure 期的状态从哪来。
