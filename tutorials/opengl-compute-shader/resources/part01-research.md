# Part 01 调研与写作范围

## 任务与范围

- 日期：2026-11-18。
- 用户明确要求：加载 OpenGL 相关 dx；删除旧 Part 02 教程；一次生成 Part 01 全部课程；补充图片；在 `resources/` 提供相关教程网址。
- 课程范围沿用已确认的 `dx:opengl-tutorial/knowledge/curriculum.md` 第 1 部，共 13 课，不再另行缩减章节。
- 读者为初学者；首次出现的概念、API 参数、数据流向、常见错误和练习需要明确说明。

## 联网调研要点

### 主流教程结构

LearnOpenGL 的 Getting Started 主线依次覆盖 OpenGL 概念、Shader/Hello Triangle、纹理、变换、坐标系统、Camera 和 Review。其 Review 页明确把顶点数据、Uniform、纹理、矩阵、三维场景与 Camera 作为入门阶段成果。Part 01 保留这一渐进主线，同时把 VBO/VAO、EBO、GLSL、Uniform 和 Depth/Culling 拆成独立课，以便首次出现时充分解释。

来源：

- [LearnOpenGL](https://learnopengl.com/)
- [LearnOpenGL：Review](https://learnopengl.com/Getting-started/Review)
- [LearnOpenGL 源码目录](https://github.com/JoeyDeVries/LearnOpenGL/tree/master/src)

### 官方语义核对

- Khronos Rendering Pipeline Overview 将 Vertex Processing、图元装配/裁剪、光栅化、Fragment Shader 和 Per-sample Processing 组织为连续管线；据此确定第 1 课的图和讲解顺序。
- Shader Compilation 与 `glCompileShader`/`glLinkProgram` 参考要求分别查询编译和链接状态；据此确定第 2 课的日志流程。
- Vertex Specification 与 Vertex Rendering 说明 VAO 记录属性格式和 Buffer 引用，索引绘制使用 VAO 中的 Element Array Buffer；据此确定第 3～4 课的绑定顺序和常见坑。
- Uniform (GLSL) 说明 Uniform 在一次绘制调用中保持一致；据此与逐顶点 Attribute 对比。
- Texture 与 Sampler Object 资料区分纹理图像、采样状态、Wrap、Filter 和 Mipmap；据此确定第 8～9 课实验。
- Face Culling 与 Rendering Pipeline Overview 区分屏幕绕序剔除和 Depth Test；据此设计第 13 课的独立开关。

来源：

- [Rendering Pipeline Overview](https://wikis.khronos.org/opengl/Rendering_Pipeline_Overview)
- [Shader Compilation](https://wikis.khronos.org/opengl/Shader_Compilation)
- [Vertex Specification](https://wikis.khronos.org/opengl/Vertex_Specification)
- [Vertex Rendering](https://wikis.khronos.org/opengl/Vertex_Rendering)
- [Uniform (GLSL)](https://wikis.khronos.org/opengl/Uniform_(GLSL))
- [Texture](https://wikis.khronos.org/opengl/Texture)
- [Sampler Object](https://wikis.khronos.org/opengl/Sampler_Object)
- [Face Culling](https://wikis.khronos.org/opengl/Face_Culling)

## 章节与成果覆盖

| 课 | 正文 | 可运行示例 | 原创图片 |
|---|---|---|---|
| 1.1 | 图形管线与状态机 | Clear Color 状态实验 | `01-01-pipeline.svg` |
| 1.2 | Shader 编译与链接 | 编译日志 + 无属性三角形 | `01-02-shader-build.svg` |
| 1.3 | VBO、VAO、Attribute | 上传位置并绘制三角形 | `01-03-vbo-vao.svg` |
| 1.4 | EBO 与索引绘制 | 索引矩形 + 线框切换 | `01-04-ebo.svg` |
| 1.5 | Hello Triangle 与插值 | 交错位置/颜色属性 | `01-05-interpolation.svg` |
| 1.6 | GLSL 基础 | 类型、Swizzle、数学函数条纹 | `01-06-glsl.svg` |
| 1.7 | Uniform、时间与交互 | 时间变色 + 键盘偏移 | `01-07-uniforms.svg` |
| 1.8 | Texture 与采样 | PPM 纹理 + Wrap/Filter 切换 | `01-08-texture.svg` |
| 1.9 | 多纹理与 Texture Unit | 两个 Sampler 混合 | `01-09-texture-units.svg` |
| 1.10 | Transform 与 GLM | T·R·S 动画 + 暂停 | `01-10-transform-order.svg` |
| 1.11 | MVP | 透视旋转立方体 | `01-11-mvp.svg` |
| 1.12 | Camera 与帧时间 | FPS Camera + deltaTime | `01-12-camera.svg` |
| 1.13 | Depth 与 Culling | 深度、绕序、剔除开关 | `01-13-depth-culling.svg` |

所有图片为本项目生成的 SVG 原理图，无外部图片版权依赖。示例纹理由项目内的二进制 P6 PPM 数据提供，可被 stb_image 直接解码。

父级图表规范默认使用 Mermaid；本次为满足“教程缺少图片”的需求，改用可离线查看、无需 Mermaid 渲染器的 SVG 图片。每课图前仍有引入文字，图后保留关键结论，因此不依赖图片渲染才能理解正文。

## 实际验证记录

- 平台：Windows 11、MSVC 19.44、CMake 4.2.1、Ninja、OpenGL 3.3 Core。
- 结果：13 个示例全部编译成功，13/13 项 CTest 隐藏窗口冒烟测试通过。
- 验证中发现并修复：第 8、9、13 课增加唯一翻译单元所需的 `STB_IMAGE_IMPLEMENTATION`，解决 stb_image 未解析符号。
- 本机重新安装 GLAD 时，vcpkg 在 `vcpkg_host_path_list(REMOVE_DUPLICATES ...)` 处失败。为隔离源码问题，本次验证复用 Part 00 已构建的 GLFW/GLAD/GLM，并从 stb 官方仓库下载 `stb_image.h` 到 `.tmp`；未修改正式教程的 Manifest 流程。因此“13 课源码构建与运行”已验证，但本机的干净 vcpkg 依赖重装未通过。

## 验证边界

- 每个示例提供 `--hidden --max-frames 2` 和 CTest 冒烟测试，用于检查配置、编译、Context、Shader 和 Draw Call 路径。
- 自动测试不替代人工视觉检查，也不模拟鼠标/键盘交互；纹理混合、线框、Camera 和 Culling 的视觉结果需按正文步骤手动确认。
- Linux、macOS 和其他驱动未在本次验证中运行，不声明已经验证。
