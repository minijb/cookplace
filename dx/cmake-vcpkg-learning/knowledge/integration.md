---
title: CMake + vcpkg 集成
brief: CMAKE_TOOLCHAIN_FILE 接入机制、manifest 模式与 CMake 构建的联动、VSCode 集成实战（presets 三件套）与 CI、交叉编译组合
tags: [cmake, vcpkg, integration, toolchain]
---
# CMake + vcpkg 集成

## 接入机制：toolchain file
官方推荐方式是把 vcpkg 自带的 `vcpkg.cmake` 作为 CMake 工具链文件：
```
cmake -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake
```
机制：工具链文件在配置期注入 vcpkg 的 `installed/<triplet>/` 到 CMake 的搜索路径链（`CMAKE_PREFIX_PATH` 等）与 `find_package` 的失败回退逻辑，使 find_package 语义不需要逐库写路径。preset 中固化该 `-D`，项目成员零心智负担。

## manifest 模式 × CMake 构建
启用 vcpkg.cmake 且项目根有 vcpkg.json 时，CMake configure 会**自动**运行 vcpkg 安装依赖到构建目录旁的 `vcpkg_installed/`（开关：`-DVCPKG_MANIFEST_INSTALL=OFF`）。即「cmake --preset 即装即建」，npm install 般体验。
- triplet 由 `VCPKG_TARGET_TRIPLET` 传入，默认 `x64-windows`（Windows）/平台默认（其他 OS）
- `VCPKG_MANIFEST_FEATURES` 可把 vcpkg.json 的 features 与 CMake option 打通（如 tests 特性只装测试依赖）

## VSCode + CMake + vcpkg 集成实战（参考配置）

2026-11-16 以 OpenGL 实验项目（glfw3 + glm + glad，MSVC/VS2022）实测跑通，并与官方 get-started-vscode 教程对齐后的「正确姿势」。三件套分工：`vcpkg.json`（manifest，项目根）声明依赖 → `CMakePresets.json` 注入 toolchain → `.vscode/settings.json` 让 CMake Tools 扩展全走 presets。

### 标准 preset 模板（VS 多配置生成器 + vcpkg）
```json
{
  "version": 6,
  "configurePresets": [
    {
      "name": "windows-msvc",
      "displayName": "Windows x64 (MSVC)",
      "generator": "Visual Studio 17 2022",
      "toolset": "host=x64",
      "architecture": "x64",
      "binaryDir": "${sourceDir}/out/build/${presetName}",
      "cacheVariables": {
        "CMAKE_INSTALL_PREFIX": "${sourceDir}/out/install/${presetName}",
        "CMAKE_TOOLCHAIN_FILE": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
        "VCPKG_TARGET_TRIPLET": "x64-windows"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "windows-msvc-debug",
      "displayName": "Windows x64 Debug",
      "configurePreset": "windows-msvc",
      "configuration": "Debug"
    }
  ]
}
```
`.vscode/settings.json`：`"cmake.useCMakePresets": "always"` + `"cmake.configureOnOpen": true`。`useCMakePresets` 取值 always/never/auto（auto 在存在 CMakePresets.json 时等价 always），显式 always 消除歧义。VCPKG_ROOT 须设为**系统级**环境变量，否则 VS Code 进程读不到。

### 实战坑位（实测发现 + 官方文档核实）
1. **toolchain 路径别写死**：`CMAKE_TOOLCHAIN_FILE` 用 `$env{VCPKG_ROOT}/...` 引环境变量；写 `C:/Users/xxx/...` 绝对路径换机即碎。`$env{}` 语法需 presets schema ≥ v3。
2. **VS 生成器不要显式设编译器**：`CMAKE_C/CXX_COMPILER: cl.exe` 对 Visual Studio 生成器无效——编译器由 toolset（v143）决定；只有 Ninja 这类命令行生成器才需要（且官方要求从 VS Developer Command Prompt 启动 VS Code 以保证 PATH）。
3. **`vcpkg_installed/` 有两个可能位置**：CMake 集成默认装进 `${CMAKE_BINARY_DIR}/vcpkg_installed`（即 `out/build/<preset>/vcpkg_installed`，官方明文，可改 `VCPKG_INSTALLED_DIR`）；在项目根手动跑 `vcpkg install` 会另生成根目录 `vcpkg_installed/`，构建并不用它——两条路径别混用，根目录残留应删。
4. **版本锁定**：manifest 无 `builtin-baseline` 时依赖版本随 ports tree 浮动；官方 `vcpkg new --application` 会生成 `vcpkg-configuration.json`（含 registry baseline），建议入版本库。学习型项目可不锁，交付项目必锁。
5. **buildPreset 的 displayName 要与 configuration 一致**：VS 多配置生成器的 build preset 必须写 `configuration`（Debug/Release）；显示名与实际配置不符属低级坑。

### 机制备忘（官方文档核实）
- toolchain file 在 `project()` 调用时求值 → 所有 vcpkg 变量必须在**首个 `project()` 之前**设置
- manifest 模式 `VCPKG_INSTALLED_DIR` 默认 `${CMAKE_BINARY_DIR}/vcpkg_installed`；classic 模式默认 `${VCPKG_ROOT}/installed`
- Windows 下 `VCPKG_APPLOCAL_DEPS` 默认 ON：构建后自动把依赖 DLL 复制到输出目录（applocal 部署惯用法由 vcpkg 代办）
- 依赖经 `find_package(<pkg> CONFIG REQUIRED)` + 命名空间 target 消费（`glm::glm`、`glad::glad`）；系统组件（`OpenGL::GL`）不走 vcpkg
- `find_package(glfw3/glm/glad)` 等第三方库的 include/link 全部交给 vcpkg 注入的搜索路径，CMakeLists 里零手写路径

## 与依赖三方案的组合边界
- vcpkg 只解决「第三方库从哪来」；项目结构仍由 CMake target 表达
- 库在 vcpkg 有 port → find_package（经 toolchain）；没有 → FetchContent/CPM；两者可混用
- 官方库没收录时：自制 port（overlay → registry）vs 直接 FetchContent 的取舍——前者可复用缓存与统一 ABI，后者上手快

## CI 组合（GitHub Actions）
要点：缓存 `vcpkg` 二进制缓存目录（GitHub Actions Cache 后端）避免每次源码重编；`lukka/run-vcpkg` 等社区 action 封装了 bootstrap + vcpkg.json 驱动的 CMake 配置；矩阵（os × triplet）各跑一条 preset。

## 交叉编译一条龙
自定义 triplet（目标平台 + sysroot/编译器）→ `-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE` 接平台工具链 → vcpkg 依赖按目标 ABI 构建 → CMake 同链生成目标构建脚本。典型场景：x64 主机交叉构建 arm64 嵌入式/移动目标。

## 待深入清单
- [ ] vcpkg.cmake 注入的完整变量/路径清单（阅读 scripts/buildsystems/vcpkg.cmake 源码）
- [x] applocal 部署已初步核实（2026-11-16）：`VCPKG_APPLOCAL_DEPS` 默认 ON，构建后自动复制依赖 DLL；遗留疑问：z-applocal 与 legacy applocal.ps1 两种实现的差异边界
- [ ] 多 triplet 共存（同项目 x64-windows 调试 + x64-windows-static 发布）的目录策略
- [ ] 与 UE 的关系：UE 自带构建系统不吃 vcpkg，vcpkg 适用于引擎外工具链/独立引擎方向 → 对齐 cpp-learning 学习目标后再深入
