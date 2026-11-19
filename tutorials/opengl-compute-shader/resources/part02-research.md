# Part 02 调研、写作范围与验证

## 当前任务与范围

- 开始日期：2026-11-19。
- 用户要求：读取 OpenGL 相关 dx，并开始 Part 02 的写作。
- 总体范围沿用此前经调研和确认的 `dx:opengl-tutorial/knowledge/curriculum.md` 第 2 部：光照、材质与模型场景，共 10 课。
- 本次实际开工范围为第 1 课“颜色、法线与光照基本量”；完整 Phong、材质结构和多光源保留给后续课程。
- 读者仍按初学者对待，首次出现的法线属性、光照空间、点积和 Normal Matrix 均提供原理、代码与实验。

## 第 1 课联网调研

### 主流教程的顺序与深入点

LearnOpenGL 的 Lighting 主线先以 Colors 建立物体颜色与光源颜色，再在 Basic Lighting 中引入 Ambient、Diffuse、Specular、法线、世界位置和 Normal Matrix，之后进入 Materials、Lighting Maps、多类光源和模型加载。这与已确认大纲一致。

本次把 Colors 与 Basic Lighting 中依赖法线的基础部分合并为第 1 课，但只实现 Diffuse，把完整 Phong 留到第 2 课，以符合当前大纲并便于单独观察每项数据。

来源：

- [LearnOpenGL：Colors](https://learnopengl.com/Lighting/Colors)
- [LearnOpenGL：Basic Lighting](https://learnopengl.com/Lighting/Basic-Lighting)
- [LearnOpenGL：Lighting Review](https://learnopengl.com/Lighting/Review)

### 官方 API 与数学语义核对

- Khronos `dot` 参考页确认其返回两个向量对应分量乘积之和；本课在两个单位向量上使用它得到夹角余弦。
- Khronos `normalize` 参考页确认其返回同方向的单位向量；本课对光线方向与插值后的法线分别归一化。
- Khronos `glUniform` 参考页用于核对 `glUniformMatrix3fv` 的 Location、Count、Transpose 和数据指针参数，以及 Matrix Uniform 的上传方式。
- Normal Matrix 的逆转置规则由 LearnOpenGL、Song Ho Ahn 的平面方程推导与正文中的切向量/法线垂直关系交叉检查；示例使用 World Space，因此矩阵来自 Model 的线性部分，而不是含 View 的矩阵。
- 静态复核发现轴对齐立方体不适合直观展示本例的 Normal Matrix 差异，因此示例改用具有斜面法线的八面体，避免 `N` 对照在归一化后看起来完全相同。

一手来源：

- [Khronos `dot`](https://registry.khronos.org/OpenGL-Refpages/gl4/html/dot.xhtml)
- [Khronos `normalize`](https://registry.khronos.org/OpenGL-Refpages/gl4/html/normalize.xhtml)
- [Khronos `glUniform`](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glUniform.xhtml)
- [OpenGL 3.3 Core Profile 规范](https://registry.khronos.org/OpenGL/specs/gl/glspec33.core.pdf)

补充来源：

- [LearnOpenGL：Basic Lighting](https://learnopengl.com/Lighting/Basic-Lighting)
- [Song Ho Ahn：Normal Vector Transformation](https://www.songho.ca/opengl/gl_normaltransform.html)

## 第 1 课产出设计

| 项目 | 内容 |
|---|---|
| 正文 | `part-02-lighting-models/01-colors-normals-lighting.md` |
| 示例 | 珊瑚色非均匀缩放八面体 + 白色光源标记 |
| 新数据 | 位置与面法线交错顶点数组；World Position 与 World Normal |
| 新 API/类型 | GLSL `dot`/`normalize`；`mat3`；`glUniformMatrix3fv` |
| 交互 | `N` 切换正确/错误 Normal Matrix；`V` 可视化法线 |
| 图片 | 原创 `02-01-lighting-basics.svg`，涵盖 RGB、漫反射与法线变换 |
| 自动测试 | 隐藏 OpenGL 3.3 Core 窗口运行两帧的 CTest 冒烟测试 |

图片沿用 Part 01 的离线 SVG 方案，而不是 Mermaid：本课需要几何方向与颜色关系图，SVG 更适合表达；图前后保留可独立理解的正文。

## 实际验证记录

- 平台：Windows 11、MSVC 19.50.35721.0、CMake 4.2.1、Ninja、OpenGL 3.3.0；显卡为 NVIDIA GeForce GTX 1660 SUPER，驱动报告版本 610.88。
- 结果：`part-02-01-lighting-basics` 干净配置、编译和链接成功；CTest 隐藏窗口冒烟测试 1/1 通过；随后直接以 `--hidden --max-frames 2` 运行也正常退出。
- 几何数值检查：8 个八面体面的叉积法线均朝外；当前非均匀缩放下，正确与错误法线方向的最大差异约为 34.06°，确认 `N` 对照具备可观察性。
- 静态检查：本地 Markdown 链接、YAML 必填字段、单一 H1、SVG XML、Python 测试脚本语法和 `git diff --check` 均通过。
- 首次默认终端选用 Clang 18.1.8，与当前 MSVC STL 要求的 Clang 19+ 不兼容。调用 `vcvars64.bat` 时又覆盖了原 `VCPKG_ROOT`，触发既有 GLAD/vcpkg 构建问题。最终显式固定 MSVC、原 vcpkg 根目录与 Ninja 后通过；这些是本次工具链环境问题，不是教程源码错误。

## 验证边界

- CTest 不验证画面美观、键盘交互或 Normal Matrix 切换的视觉差异。
- 非 Windows 平台与不同显卡驱动需在相应环境另行验证。
- 本课颜色计算是教学用简化模型，不宣称实现完整物理光照、Gamma Correction 或距离衰减。
