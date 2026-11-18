# OpenGL 与 Compute Shader 示例代码

每课示例都有独立的 `CMakeLists.txt`、`CMakePresets.json` 和 `vcpkg.json`，可分别配置、构建和测试。示例使用 C++17、CMakePresets v3 和 vcpkg Manifest Mode。

## 通用准备

1. 安装 CMake 3.21+、Ninja 和支持 C++17 的编译器。
2. 安装 vcpkg，并设置 `VCPKG_ROOT` 为其根目录。
3. Windows + MSVC 下使用已初始化编译器环境的终端；若使用 Clang，需确保版本受当前 Visual Studio STL 支持。
4. 窗口示例需要可用的显卡驱动。Part 00 第 7 课需要 OpenGL 4.3 Core Debug Context；Part 01 全部示例使用 OpenGL 3.3 Core。
5. 正常运行需要图形环境；CTest 使用隐藏窗口，但仍需要能创建 OpenGL Context，并非纯 CPU 测试。

## Part 00 示例

| 目录 | 对应课程 | 依赖 | 测试内容 |
|---|---|---|---|
| `part-00-03-window` | 第 0 部第 3 课：窗口、Context 与主循环 | GLFW、GLAD、系统 OpenGL | 隐藏 3.3 Core 窗口运行两帧 |
| `part-00-04-project` | 第 0 部第 4 课：CMake 工程 | GLFW、GLAD、系统 OpenGL | 完整配置、构建和窗口冒烟测试 |
| `part-00-05-math` | 第 0 部第 5 课：数学基础 | GLM | 向量、矩阵、齐次坐标数值断言 |
| `part-00-06-coordinates` | 第 0 部第 6 课：坐标空间 | GLM | MVP、NDC、Near/Far Plane、Viewport 断言 |
| `part-00-07-debug` | 第 0 部第 7 课：Debug Output | GLFW、GLAD、系统 OpenGL | 正常 Debug Context 与故意错误路径 |

## Part 01 示例

| 目录 | 可执行文件 | 正常结果与交互 |
|---|---|---|
| `part-01-01-pipeline` | `part01_01_pipeline` | `1`/`2`/`3` 切换 Clear Color |
| `part-01-02-shaders` | `part01_02_shaders` | Shader 编译链接；无属性三角形 |
| `part-01-03-vbo-vao` | `part01_03_vbo_vao` | VBO/VAO 位置属性三角形 |
| `part-01-04-ebo` | `part01_04_ebo` | 索引矩形；`L` 切换线框 |
| `part-01-05-interpolation` | `part01_05_interpolation` | 位置与颜色交错属性；插值三角形 |
| `part-01-06-glsl` | `part01_06_glsl` | GLSL 数学函数生成条纹 |
| `part-01-07-uniforms` | `part01_07_uniforms` | 方向键移动；`R` 复位；时间变色 |
| `part-01-08-textures` | `part01_08_textures` | `F` 切换 Filter；`W` 切换 Wrap |
| `part-01-09-multitexture` | `part01_09_multitexture` | 两个 Texture Unit；上下键调整混合 |
| `part-01-10-transform` | `part01_10_transform` | T·R·S 动画；空格暂停 |
| `part-01-11-mvp` | `part01_11_mvp` | MVP 与透视旋转立方体 |
| `part-01-12-camera` | `part01_12_camera` | 鼠标观察；WASD 移动；帧时间 |
| `part-01-13-depth-culling` | `part01_13_depth_culling` | `D` 深度、`C` 剔除、`F` 绕序开关 |

所有窗口示例按 `Esc` 退出。Part 01 的共享代码和资源：

- `part-01-common/gl_common.hpp`：窗口、参数解析、Shader 编译链接、Uniform 和立方体几何。
- `part-01-common/texture_common.hpp`：stb_image 解码与二维纹理上传。
- `part-01-assets/`：棋盘和渐变 PPM 测试纹理。
- 共享代码不属于额外一课；每课工程通过相对 Include 路径复用它，仍有独立构建与测试入口。

## 构建任意示例

进入某个示例目录，例如：

```powershell
cd tutorials/opengl-compute-shader/code/part-01-03-vbo-vao
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

其他示例使用相同三条 CMake/CTest 命令。构建产物位于各示例的 `build/debug/`，已被 `.gitignore` 排除。

从仓库根目录一次验证全部 Part 01 示例：

```powershell
python tutorials/opengl-compute-shader/code/test-part01.py
```

需要清理后重建：

```powershell
python tutorials/opengl-compute-shader/code/test-part01.py --clean
```

脚本要求终端已初始化 C++ 编译器环境，并正确设置 `VCPKG_ROOT`。任一工程失败时脚本会停止并保留错误日志。

## 手动运行

Windows 下进入对应示例目录后运行：

```powershell
.\build\debug\<目标名>.exe
```

例如：

```powershell
cd tutorials/opengl-compute-shader/code/part-01-12-camera
.\build\debug\part01_12_camera.exe
```

Linux 下通常去掉 `.exe`。所有 Part 01 示例也支持：

```powershell
.\build\debug\<目标名>.exe --hidden --max-frames 2
```

该模式用于自动冒烟测试。第 8、9、13 课通过 CMake 编译定义获得纹理资产绝对路径，因此从构建目录启动 CTest 也能加载图像。

## 验证边界

- CTest 验证配置、编译、Context 创建、Shader 编译链接和 Draw Call 能完成。
- 隐藏窗口测试不验证画面美观，也不模拟鼠标、键盘和不同窗口尺寸。
- 纹理方向、插值、Camera 手感、Depth/Culling 的视觉差异请按课文手动检查。
- 具体平台验证结果见 [Part 00 记录](../resources/part00-remaining-research.md)与 [Part 01 调研记录](../resources/part01-research.md)。
