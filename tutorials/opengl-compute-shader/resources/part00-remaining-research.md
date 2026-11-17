---
title: Part 00 第三至第七章调研与验证记录
series: opengl-compute-shader
topic: 窗口、工程、数学、坐标空间和 Debug Output 五章的来源与可运行示例验证
audience: 教程作者与审校者
outcome: 追溯 Part 00 剩余课程的范围、外部依据、示例代码和实际测试状态
environment: 在线资料调研；Windows 11 示例验证环境见验证状态；2026-11-16
date: 2026-11-16
---
# Part 00 第三至第七章调研与验证记录

## 范围确认

- 用户在前两章完成后明确要求“part00 一起吧”，据此一次完成第 0 部剩余第 3～7 章。
- 课程范围沿用 `dx:opengl-tutorial/knowledge/curriculum.md`，不改变既定章节顺序。
- 第 3、4、7 章使用可自动关闭的真实 OpenGL Context 示例；第 5、6 章使用无需 GPU 的 GLM 数值示例。
- 不把三角形、Shader、Camera 控制或 RenderDoc 提前塞入 Part 00；它们属于后续大纲。

## 第三章：窗口、Context 与主循环

### 调研要点

1. GLFW 官方 Quick Guide 要求通过 Window Hint 请求最低 OpenGL 版本和 Core Profile，并检查窗口/Context 创建是否失败。
2. Khronos Getting Started 将初始化分为 Context 创建与函数加载；GLAD 必须在当前 Context 建立后加载函数。
3. GLFW 默认双缓冲，`glfwSwapBuffers` 交换前后 Buffer；持续渲染程序需要定期 `glfwPollEvents`。
4. Viewport 应使用 Framebuffer 尺寸并在尺寸改变时更新。

### 来源

- [GLFW Getting Started](https://www.glfw.org/docs/latest/quick_guide.html)：已检索并读取摘要，2026-11-16。
- [GLFW Window Guide](https://www.glfw.org/docs/latest/window.html)：已检索并读取摘要，2026-11-16。
- [GLFW Window Reference](https://www.glfw.org/docs/latest/group__window.html)：已检索并读取摘要，2026-11-16。
- [GLAD 官方仓库](https://github.com/Dav1dde/glad)：已检索并读取摘要，2026-11-16；用于区分 GLAD 1/2。
- [Khronos Getting Started](https://wikis.khronos.org/opengl/Getting_Started)：已检索并读取摘要，2026-11-16。

## 第四章：CMake 工程与依赖

### 调研要点

1. CMake 通过 Target 组织源码、编译设置和链接依赖；第三方包通过 `find_package(... CONFIG REQUIRED)` 接入。
2. vcpkg Manifest Mode 用 `vcpkg.json` 声明依赖，并通过 CMake Toolchain 在配置阶段安装到构建树。
3. CMakePresets 保存生成器、构建目录、Toolchain 和构建类型，减少隐含命令行状态。
4. 本工作区已有 GLFW + GLAD + GLM 的 vcpkg 基线，可复用相同包名、Target 和 `builtin-baseline`。

### 来源

- [CMake Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/)：官方工程与 Target 基础。
- [CMake `cmake-presets` 手册](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)：官方 Preset 语义。
- [CMake `find_package` 手册](https://cmake.org/cmake/help/latest/command/find_package.html)：官方依赖查找语义。
- [vcpkg Manifest Mode](https://learn.microsoft.com/vcpkg/concepts/manifest-mode)：Microsoft Learn 官方 vcpkg 文档。
- [vcpkg CMake Integration](https://learn.microsoft.com/vcpkg/users/buildsystems/cmake-integration)：Toolchain 集成。
- 本地已验证工程 `tutorials/cmake-vcpkg/code/part-03-02-opengl/`：复用 vcpkg 基线与 GLAD 1 包配置。

## 第五章：数学基础

### 调研要点

1. GLM 提供 `vec3`、`mat4`、几何函数与矩阵变换；默认函数使用 OpenGL 风格约定。
2. 点积同时包含长度与夹角信息，只有单位向量点积可直接作为夹角余弦。
3. 叉积顺序决定方向；右手基底下 `x × y = z`。
4. 齐次坐标统一矩阵表达，`w = 1` 的点受平移，`w = 0` 的方向不受平移。
5. 矩阵乘法顺序影响结果；本课程使用矩阵乘列向量约定。

### 来源

- [GLM 官方手册](https://github.com/g-truc/glm/blob/master/manual.md)：已检索并读取摘要，2026-11-16。
- [GLM `matrix_transform` 源码文档](https://github.com/g-truc/glm/blob/master/glm/ext/matrix_transform.hpp)：已检索并读取摘要，2026-11-16。
- [Khronos Coordinate Transformations](https://wikis.khronos.org/opengl/Coordinate_Transformations)：已检索并读取摘要，2026-11-16。
- [LearnOpenGL Transformations](https://learnopengl.com/Getting-Started/Transformations)：第三方教学顺序参考。

## 第六章：坐标空间

### 调研要点

1. 坐标链为 Local → World → View → Clip → NDC → Window；分别由 Model、View、Projection、透视除法和 Viewport 连接。
2. `lookAtRH` 使用右手 View 约定；Camera 前方为 View Space 负 Z。
3. `perspectiveRH_NO` 对应右手投影与 OpenGL `-1..1` NDC 深度范围。
4. Clip Space 需要除以 `w` 才成为 NDC；Viewport 再把 NDC 映射到 Framebuffer 区域。
5. 用户反馈对坐标转换/映射不理解，因此本章后续补充坐标系三要素、转换与移动的区别、NDC/Window 双向映射、完整数值示例和逐空间排查方法；新增内容沿用上述官方坐标变换与管线来源。

### 来源

- [Khronos Coordinate Transformations](https://wikis.khronos.org/opengl/Coordinate_Transformations)：已检索并读取摘要，2026-11-16。
- [Khronos Rendering Pipeline Overview](https://wikis.khronos.org/opengl/Rendering_Pipeline_Overview)：已检索并读取摘要，2026-11-16。
- [GLM 官方手册](https://github.com/g-truc/glm/blob/master/manual.md)：Handedness 与深度范围配置。
- [LearnOpenGL Coordinate Systems](https://learnopengl.com/Getting-Started/Coordinate-Systems)：已检索并读取摘要，2026-11-16。

## 第七章：Debug Output

### 调研要点

1. Debug Output 自 OpenGL 4.3 起进入 Core；早期 `KHR_debug`/`ARB_debug_output` 不与本例混用。
2. `glDebugMessageCallback` 注册回调；`GL_DEBUG_OUTPUT_SYNCHRONOUS` 提供同线程、触发调用范围内的回调保证。
3. 消息包含 Source、Type、ID、Severity 和文本；具体驱动消息内容不作为跨平台固定断言。
4. `glObjectLabel`、Debug Group 和 `glDebugMessageInsert` 分别提供对象名称、调用分组和应用 Marker。
5. 示例用 `glEnable(0)` 故意产生 `GL_INVALID_ENUM`，验证回调路径，而不只测试程序是否崩溃。

### 来源

- [Khronos Debug Output](https://wikis.khronos.org/opengl/Debug_Output)：已检索并读取摘要，2026-11-16。
- [OpenGL 4 Reference：`glDebugMessageCallback`](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDebugMessageCallback.xhtml)：官方 API 参考。
- [OpenGL 4 Reference：`glDebugMessageInsert`](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDebugMessageInsert.xhtml)：官方 API 参考。
- [GLFW Window Guide](https://www.glfw.org/docs/latest/window.html)：Debug Context Hint。
- [Khronos OpenGL Error](https://wikis.khronos.org/opengl/OpenGL_Error)：错误状态与 `glGetError`。

## 示例结构检查

- 五个代码目录都包含独立 `CMakeLists.txt`、`CMakePresets.json` 和 `vcpkg.json`。
- 所有 C++ Target 使用 C++17，并关闭编译器语言扩展。
- 窗口示例定义 `GLFW_INCLUDE_NONE`，先包含 GLAD，再包含 GLFW。
- 所有示例注册 CTest；窗口测试使用隐藏窗口与固定帧数，不需要人工关闭。
- 数学示例使用误差比较和显式失败退出，不依赖 Release 下会被禁用的 `assert`。
- 调试示例保留正常和故意错误两条测试路径。

## 验证状态

### 最终环境

- Windows 11；CMake 4.2.1；Ninja 1.13.2。
- MSVC 19.50.35721（Visual Studio 18）；Windows SDK 10.0.26100.0。
- vcpkg 基线 `91f0baffae4e06a68a961d7f6493e8a1ca6775eb`。
- GLFW 3.5.1；GLAD 0.1.36；GLM 1.0.3。
- OpenGL 窗口测试使用 NVIDIA 610.88 驱动，第七章实际获得 OpenGL 4.3.0 Debug Context。

### 最终测试结果

| 示例 | Configure / Build | CTest |
|---|---|---|
| 第三章 `ch03_window` | 通过 | `ch03_window_smoke` 通过 |
| 第四章 `ch04_project` | 通过 | `ch04_project_smoke` 通过 |
| 第五章 `ch05_math` | 通过 | `ch05_math_examples` 通过 |
| 第六章 `ch06_coordinates` | 通过 | `ch06_coordinate_examples` 通过 |
| 第七章 `ch07_debug` | 通过 | 正常与故意错误两项测试均通过 |

第七章额外检查 `LastTest.log`：正常路径没有 OpenGL Error；故意错误路径实际捕获 `GL_INVALID_ENUM`。CTest 对正常路径配置 Error 失败正则，对故意错误路径配置必需 `GL_INVALID_ENUM` 和非预期 `GL_INVALID_VALUE` 检查，不再只依赖进程退出码。

### 验证过程中修正的问题

- 初次 Ninja 配置选中 Clang 18.1.8，与当前 Visual Studio STL 不兼容；最终使用受支持的 MSVC，并显式补齐 Ninja、Windows SDK 和用户 vcpkg 路径。
- 第五章测试发现两种矩阵顺序的预期值漏写 Y/Z 平移；正文和代码已统一改为 `(12,20,30)` 与 `(22,40,60)`。
- 第六章测试发现手算 NDC 预期值时漏算透视常数项的数量级；已按公式修正并通过 Near/Far Plane 等全部断言。
- 第六章扩充坐标映射内容时，新增 Local → Window 的完整数值示例及 Window → NDC 反向映射断言，复验要求见下方补充记录。

### 第六章映射内容补充验证

2026-11-17 使用同一 Windows 11 + MSVC 环境重新构建并运行 `ch06_coordinate_examples`，CTest 通过。新增断言覆盖完整链各阶段、`clip.w`、NDC `(0.603553,0,0.935269)`、窗口坐标 `(641.4213,300)` 以及 Window → NDC 反向映射。
- 第七章日志审查发现 Buffer 在实例化前设置标签会产生 `GL_INVALID_VALUE`；已改为绑定并分配存储后再调用 `glObjectLabel`，同时增强日志断言。

其他操作系统、GPU、GCC 和 Clang 工具链未在本次验证中覆盖，正文不作已验证声明。
