# Part 00 示例代码

每个目录都是独立的 CMake + vcpkg 工程，可单独配置、构建和测试。示例使用 C++17、CMakePresets v3 和 vcpkg Manifest Mode。

## 通用准备

1. 安装 CMake 3.21+、Ninja 和支持 C++17 的编译器。
2. 安装 vcpkg，并设置 `VCPKG_ROOT` 为其根目录。
3. Windows + MSVC 下使用已初始化编译器环境的终端；若使用 Clang，需确保其版本受当前 Visual Studio STL 支持。
4. OpenGL 窗口示例需要可用显卡驱动；第七章需要 OpenGL 4.3 Core Debug Context。

## 示例列表

| 目录 | 对应课程 | 依赖 | 测试内容 |
|---|---|---|---|
| `part-00-03-window` | 第三章：窗口、Context 与主循环 | GLFW、GLAD、系统 OpenGL | 隐藏 3.3 Core 窗口运行两帧 |
| `part-00-04-project` | 第四章：CMake 工程 | GLFW、GLAD、系统 OpenGL | 完整配置、构建和窗口冒烟测试 |
| `part-00-05-math` | 第五章：数学基础 | GLM | 向量、矩阵、齐次坐标数值断言 |
| `part-00-06-coordinates` | 第六章：坐标空间 | GLM | MVP、NDC、Near/Far Plane、Viewport 断言 |
| `part-00-07-debug` | 第七章：Debug Output | GLFW、GLAD、系统 OpenGL | 正常 Debug Context 与故意错误路径 |

## 构建任意示例

进入某个示例目录，例如：

```powershell
cd tutorials/opengl-compute-shader/code/part-00-03-window
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

其他示例使用相同三条 CMake/CTest 命令。构建产物位于各示例的 `build/debug/`，已被 `.gitignore` 排除。

## 手动运行

Windows 下从对应示例目录运行：

```powershell
# 第三章
.\build\debug\ch03_window.exe

# 第四章
.\build\debug\ch04_project.exe

# 第五章
.\build\debug\ch05_math.exe

# 第六章
.\build\debug\ch06_coordinates.exe

# 第七章：正常路径
.\build\debug\ch07_debug.exe

# 第七章：故意触发一次 GL_INVALID_ENUM
.\build\debug\ch07_debug.exe --trigger-error
```

窗口程序按 ESC 退出。第三章、第四章和第七章还支持 `--hidden --max-frames 2`，用于不显示窗口的自动冒烟测试。

实际验证平台和结果见 [Part 00 调研与验证记录](../resources/part00-remaining-research.md)。
