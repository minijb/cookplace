# OpenGL 学习资料与教程网址

本目录保存课程的调研、验证记录和扩展阅读。以下网址按用途分类；官方规范用于核对行为，第三方教程用于补充讲解和练习。外部页面可能更新，遇到差异时以当前 OpenGL 版本规范和实际驱动为准。

## 系统教程

- [LearnOpenGL](https://learnopengl.com/)：现代 OpenGL 系统教程，覆盖入门、光照、模型、高级渲染与 PBR。
- [LearnOpenGL：Getting started](https://learnopengl.com/Getting-started/OpenGL)：Part 01 的主要补充读物，可按左侧目录逐章学习。
- [LearnOpenGL 示例源码](https://github.com/JoeyDeVries/LearnOpenGL/tree/master/src)：对照各章完整 C++ 示例。
- [OpenGL Tutorial](https://www.opengl-tutorial.org/)：从三角形、矩阵到模型和着色效果的另一套图形教程。
- [OGLdev](https://ogldev.org/)：现代 OpenGL 分课教程，含插值、变换、纹理和光照专题。
- [Anton's OpenGL 4 Tutorials](https://antongerdelan.net/opengl/)：偏实践的 OpenGL 4 教程与后续 Compute Shader 内容。
- [Song Ho Ahn：OpenGL](http://www.songho.ca/opengl/)：矩阵、投影、纹理和固定功能的直观说明，适合补充数学图示。

## 官方规范与 API 参考

- [Khronos OpenGL Registry](https://registry.khronos.org/OpenGL/index_gl.php)：各版本 OpenGL 与 GLSL 规范入口。
- [OpenGL 3.3 Core Profile 规范](https://registry.khronos.org/OpenGL/specs/gl/glspec33.core.pdf)：本系列基础课程的版本依据。
- [Khronos OpenGL Wiki](https://wikis.khronos.org/opengl/Main_Page)：对象模型、管线和 API 语义参考。
- [Khronos Rendering Pipeline Overview](https://wikis.khronos.org/opengl/Rendering_Pipeline_Overview)：图形管线与各处理阶段。
- [Khronos Shader Compilation](https://wikis.khronos.org/opengl/Shader_Compilation)：Shader 编译、链接与 Program。
- [Khronos Vertex Specification](https://wikis.khronos.org/opengl/Vertex_Specification)：VAO、VBO 和顶点属性格式。
- [Khronos Vertex Rendering](https://wikis.khronos.org/opengl/Vertex_Rendering)：`glDrawArrays`、EBO 与 `glDrawElements`。
- [Khronos Texture](https://wikis.khronos.org/opengl/Texture)：Texture Object、上传与采样。
- [Khronos Sampler Object](https://wikis.khronos.org/opengl/Sampler_Object)：Filter、Wrap 和采样状态。
- [Khronos Uniform (GLSL)](https://wikis.khronos.org/opengl/Uniform_(GLSL))：Uniform 语义与设置。
- [Khronos Face Culling](https://wikis.khronos.org/opengl/Face_Culling)：Winding、Front Face 和 Culling。
- [docs.gl](https://docs.gl/)：按 OpenGL 版本检索 API 的便捷参考；最终仍可与规范交叉核对。

## 工具与依赖文档

- [GLFW 文档](https://www.glfw.org/docs/latest/)：窗口、Context、输入和时间。
- [GLFW 输入指南](https://www.glfw.org/docs/latest/input_guide.html)：键盘、鼠标和 Cursor 模式。
- [GLAD 生成器](https://glad.dav1d.de/)：生成 OpenGL 函数加载器；本课程示例通过 vcpkg 安装。
- [GLM 手册](https://github.com/g-truc/glm/blob/master/manual.md)：向量、矩阵、变换和类型布局。
- [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h)：图像解码 API、支持格式与注意事项。
- [RenderDoc](https://renderdoc.org/)：帧捕获与图形调试工具，适合后续检查 Buffer、Texture 和 Draw Call。

## Part 01 逐课补充阅读

| 课 | 推荐阅读 |
|---|---|
| 1.1 管线与状态机 | [LearnOpenGL：OpenGL](https://learnopengl.com/Getting-started/OpenGL)、[Rendering Pipeline Overview](https://wikis.khronos.org/opengl/Rendering_Pipeline_Overview) |
| 1.2 Shader 编译链接 | [LearnOpenGL：Shaders](https://learnopengl.com/Getting-started/Shaders)、[Shader Compilation](https://wikis.khronos.org/opengl/Shader_Compilation) |
| 1.3～1.5 VBO/VAO/EBO/三角形 | [LearnOpenGL：Hello Triangle](https://learnopengl.com/Getting-started/Hello-Triangle)、[Vertex Specification](https://wikis.khronos.org/opengl/Vertex_Specification) |
| 1.6～1.7 GLSL 与 Uniform | [LearnOpenGL：Shaders](https://learnopengl.com/Getting-started/Shaders)、[OpenGL Shading Language](https://wikis.khronos.org/opengl/OpenGL_Shading_Language) |
| 1.8～1.9 纹理 | [LearnOpenGL：Textures](https://learnopengl.com/Getting-started/Textures)、[Sampler Object](https://wikis.khronos.org/opengl/Sampler_Object) |
| 1.10～1.11 变换与 MVP | [LearnOpenGL：Transformations](https://learnopengl.com/Getting-started/Transformations)、[Coordinate Systems](https://learnopengl.com/Getting-started/Coordinate-Systems) |
| 1.12 Camera | [LearnOpenGL：Camera](https://learnopengl.com/Getting-started/Camera)、[GLFW 输入指南](https://www.glfw.org/docs/latest/input_guide.html) |
| 1.13 Depth 与 Culling | [LearnOpenGL：Depth Testing](https://learnopengl.com/Advanced-OpenGL/Depth-testing)、[Face Culling](https://learnopengl.com/Advanced-OpenGL/Face-culling) |

## Part 02 逐课补充阅读

| 课 | 推荐阅读 |
|---|---|
| 2.1 颜色、法线与光照基本量 | [LearnOpenGL：Colors](https://learnopengl.com/Lighting/Colors)、[LearnOpenGL：Basic Lighting](https://learnopengl.com/Lighting/Basic-Lighting)、[Song Ho Ahn：Normal Vector Transformation](https://www.songho.ca/opengl/gl_normaltransform.html) |

## 后续 Compute Shader 预习

- [Khronos Compute Shader](https://wikis.khronos.org/opengl/Compute_Shader)：Dispatch、Workgroup、Shared Memory 和同步概念。
- [Anton's OpenGL 4 Tutorials：Compute Shaders](https://antongerdelan.net/opengl/compute.html)：Compute 最小示例。
- [Khronos：How to Use and Teach OpenGL Compute Shaders](https://www.khronos.org/assets/uploads/developers/library/2014-siggraph-bof/KITE-BOF_Aug14.pdf)：Compute 教学顺序参考。

## 本目录记录

- [Part 01 调研与写作范围](part01-research.md)：课程来源、章节覆盖、图像与验证说明。
- [Part 02 调研、进度与验证](part02-research.md)：光照与模型课程的来源、已完成课程和验证边界。
- [第 0 部第 1 课调研](ch01-research.md)。
- [第 0 部第 2 课调研](ch02-research.md)。
- [第 0 部第 3～7 课调研与验证](part00-remaining-research.md)。
