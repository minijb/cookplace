---
title: OpenGL 与 Compute Shader 系统教学大纲
brief: 从基础快速重建、进阶渲染和现代 OpenGL 工程实践，到 Compute Shader 执行模型、同步与综合项目的完整有序课程表
tags: [opengl, graphics, glsl, compute-shader, curriculum]
---
# OpenGL 与 Compute Shader 系统教学大纲

## 读者画像与完成目标

读者按 **OpenGL 与 Compute Shader 初学者** 对待，不预设已有图形学、OpenGL 或并行计算经验。学习策略是“基础讲清楚、循序渐进、重实践、再进入进阶内容”，不能因后续包含高级主题而压缩必要的基础解释。

### 讲解详细程度（写作要求）

后续依据本大纲撰写教程时，整体需要**稍微详细一些**，至少遵守以下要求：

- 新概念首次出现时，先说明“是什么、为什么需要、在本课中怎么用”，再给出代码。
- 不默认读者记得前置知识；涉及较早课程时，用一句话回顾并链接对应课程。
- 首次出现的 OpenGL/GLSL API 要解释用途、关键参数、常见返回值或错误，以及调用前后的状态变化。
- 关键代码按数据流向分步讲解，说明 CPU 数据如何进入 GPU、各 Shader 阶段如何传递数据、结果写到哪里。
- 数学与并行计算概念配合直观例子或小规模演算，不只给公式和术语。
- 每课提供可运行的最小示例、预期结果、常见错误和独立练习；不能只有代码片段而没有上下文。
- “稍微详细”不等于重复堆砌：相同背景不反复铺陈，深入数学证明和冷门扩展放入选学内容。

完成全部主线后，应能够：

- 独立搭建现代 OpenGL 工程，解释数据如何经过图形管线成为屏幕图像。
- 实现模型场景、阴影、后处理、延迟渲染和基于物理的渲染（PBR）。
- 使用调试与性能工具定位黑屏、状态污染、同步错误和性能瓶颈。
- 从零编写、Dispatch 并验证 Compute Shader，正确使用 SSBO、Image、Shared Memory、Barrier 和 Atomic。
- 让 Compute Shader 与渲染管线安全协作，完成 GPU 粒子或 GPU 剔除综合项目。

## 环境与使用方式

- 主语言为 C++17；建议使用 CMake、GLFW、GLAD、GLM，图像和模型加载可分别使用 stb_image 与 Assimp。
- 第 1～5 部以 OpenGL 3.3 Core Profile 为共同基线；明确标注需要更高版本的功能。
- 第 6～10 部使用 OpenGL 4.3 Core Profile；DSA 等专题使用 OpenGL 4.5。
- 每课采用“背景与目的 → 原理 → 最小示例 → 逐步讲解 → 修改实验 → 常见错误 → 独立练习”的结构。
- 每个里程碑先独立实现，再对照答案；只能运行复制代码不算完成。
- macOS 的 OpenGL 支持限制会影响 4.3 及以上课程，开始 Compute Shader 前必须检查实际平台与驱动能力。

## 总体路线

```mermaid
flowchart LR
    A[基础诊断与环境] --> B[OpenGL 基础重建]
    B --> C[光照与模型]
    C --> D[高级 OpenGL]
    D --> E[高级渲染与 PBR]
    E --> F[现代 API、调试与性能]
    F --> G[Compute Shader 基础]
    G --> H[资源、同步与并行算法]
    H --> I[Compute 与渲染综合项目]
```

## 第 0 部：定位、环境与必要数学

| 课 | 内容 | 必须掌握或产出 |
|---|---|---|
| 0.1 | 学前准备与课程导览：三角形、Shader、Texture、MVP、Camera | 了解后续会学什么；自测仅用于发现陌生概念，不作为学习门槛 |
| 0.2 | 现代 OpenGL 与立即模式、Core/Compatibility Profile | 理解为何课程不使用固定管线和旧式 `glBegin` |
| 0.3 | 窗口、Context、函数加载与主循环 | 建立 GLFW + GLAD 最小工程并打印版本信息 |
| 0.4 | CMake 工程、目录结构与跨平台依赖 | 可从干净目录配置、构建和运行 |
| 0.5 | 向量、矩阵、点积、叉积与齐次坐标 | 能手算小规模变换并解释矩阵乘法顺序 |
| 0.6 | 坐标空间与右手系 | 区分 Local、World、View、Clip、Screen Space |
| 0.7 | Debug Output、错误检查与开发日志 | 建立后续课程统一使用的错误报告机制 |

**里程碑**：运行一个可调整 Clear Color、能报告 OpenGL 错误的工程；通过基础诊断表。

## 第 1 部：OpenGL 基础快速重建

| 课 | 内容 | 必须掌握或产出 |
|---|---|---|
| 1.1 | 图形管线全景与 OpenGL 状态机 | 从顶点数据追踪到 Fragment 与 Framebuffer |
| 1.2 | Vertex Shader、Fragment Shader 编译与链接 | 能读取编译/链接日志并定位常见错误 |
| 1.3 | VBO、VAO、Vertex Attribute | 解释数据存储、解释格式和绘制状态各自职责 |
| 1.4 | EBO 与索引绘制 | 比较 `glDrawArrays` 与 `glDrawElements` |
| 1.5 | Hello Triangle 与插值 | 独立绘制彩色三角形并解释插值结果 |
| 1.6 | GLSL 基础：类型、限定符、内置函数 | 复习 `in/out`、`uniform`、向量分量与常用数学函数 |
| 1.7 | Uniform、时间与交互 | 用 Uniform 控制颜色、位置或动画 |
| 1.8 | Texture、Sampler、Wrap、Filter 与 Mipmap | 显示纹理并解释采样参数与上下方向问题 |
| 1.9 | 多纹理与 Texture Unit | 混合两张纹理，理解 Sampler 与纹理单元绑定 |
| 1.10 | Transform 与 GLM | 完成平移、旋转、缩放组合实验 |
| 1.11 | Model、View、Projection | 构建 MVP 并解释透视除法和裁剪 |
| 1.12 | Camera、输入与帧时间 | 实现 FPS Camera，运动速度不随帧率改变 |
| 1.13 | Depth Test、Face Winding 与 Culling | 正确渲染实心立方体，诊断遮挡和面消失问题 |

**里程碑**：带纹理立方体场景，可移动 Camera、切换 Depth Test/Culling，并观察结果。

## 第 2 部：光照、材质与模型场景

| 课 | 内容 | 必须掌握或产出 |
|---|---|---|
| 2.1 | 颜色、法线与光照基本量 | 理解法线变换及非均匀缩放带来的问题 |
| 2.2 | Phong：Ambient、Diffuse、Specular | 分分量显示并最终合成光照 |
| 2.3 | Material 与 Light 属性 | 将材质和光源参数组织为可复用结构 |
| 2.4 | Diffuse/Specular Map | 使用纹理控制材质光照响应 |
| 2.5 | Directional、Point、Spot Light | 实现三类光源及衰减、平滑边缘 |
| 2.6 | 多光源与 Uniform 数据组织 | 在一个 Shader 中处理多个光源 |
| 2.7 | Assimp 与模型数据导入 | 理解 Node、Mesh、Material、Texture 的关系 |
| 2.8 | Mesh/Model 抽象与资源生命周期 | 完成可复用模型加载与绘制接口 |
| 2.9 | 场景组织与 Transform 层级 | 渲染多个模型并正确组合父子变换 |
| 2.10 | 资源路径、坐标系与资产常见问题 | 能诊断贴图丢失、尺度异常、模型翻转 |

**里程碑**：加载外部模型，使用材质贴图和多种光源搭建可交互场景。

## 第 3 部：高级 OpenGL 与离屏渲染

| 课 | 内容 | 必须掌握或产出 |
|---|---|---|
| 3.1 | Depth Buffer 精度、可视化与 Z-fighting | 解释非线性深度并处理深度冲突 |
| 3.2 | Stencil Test | 实现轮廓或遮罩效果 |
| 3.3 | Blending 与透明排序 | 解释透明物体顺序和常见伪影 |
| 3.4 | Framebuffer、Renderbuffer 与 Texture Attachment | 创建并验证完整 FBO |
| 3.5 | 离屏渲染与 Post-processing | 实现反相、模糊、 sharpen 等屏幕效果 |
| 3.6 | Cubemap 与 Skybox | 创建天空盒并处理深度和方向采样 |
| 3.7 | Environment Mapping | 实现基础反射与折射 |
| 3.8 | 高级 Buffer 数据与更新策略 | 比较 `glBufferData`、`glBufferSubData` 与映射 |
| 3.9 | Uniform Buffer Object（UBO） | 共享 Matrices 等数据，理解标准布局 |
| 3.10 | Instancing | 用一次 Draw Call 渲染大量实例 |
| 3.11 | Geometry Shader | 理解输入输出图元及适用边界，完成小型效果 |
| 3.12 | Tessellation Shader（OpenGL 4.0+） | 理解 Patch、TCS、TES 和细分参数 |
| 3.13 | MSAA 与抗锯齿基础 | 比较多重采样和后处理抗锯齿 |
| 3.14 | Gamma Correction 与颜色空间 | 避免在错误颜色空间中进行光照计算 |

**里程碑**：包含 Skybox、透明物体、Instancing 和可选择后处理的离屏渲染场景。

## 第 4 部：高级光照、阴影与 PBR

| 课 | 内容 | 必须掌握或产出 |
|---|---|---|
| 4.1 | Blinn-Phong | 比较 Phong 与 Blinn-Phong 的高光表现 |
| 4.2 | Shadow Mapping 原理与 Depth Pass | 从光源视角生成并使用 Shadow Map |
| 4.3 | Shadow Acne、Peter Panning 与 PCF | 诊断并改善阴影偏移、悬空和锯齿 |
| 4.4 | Omnidirectional Shadow Mapping | 使用 Cubemap 实现点光源阴影 |
| 4.5 | Cascaded Shadow Maps | 按视锥分区改善方向光阴影分辨率 |
| 4.6 | Normal Mapping 与 TBN | 在 Tangent Space 实现细节法线 |
| 4.7 | Parallax Mapping | 实现并比较基础、陡峭和遮蔽视差映射 |
| 4.8 | HDR、Exposure 与 Tone Mapping | 使用浮点 Framebuffer 完成色调映射 |
| 4.9 | Bloom | 实现亮度提取、模糊和合成链路 |
| 4.10 | Deferred Shading 与 G-buffer | 分离几何和光照 Pass，比较 Forward Rendering |
| 4.11 | SSAO | 使用 Position、Normal、Noise 和 Kernel 实现环境光遮蔽 |
| 4.12 | PBR 理论与 Cook-Torrance BRDF | 理解 Metallic、Roughness、Fresnel、NDF、Geometry |
| 4.13 | Direct PBR Lighting | 实现直接光 PBR 并验证材质参数 |
| 4.14 | IBL：Irradiance、Prefilter、BRDF LUT | 完成漫反射与镜面环境光 |

**里程碑**：可切换 Forward/Deferred 的 PBR 场景，包含阴影、HDR、Bloom 和 IBL。

## 第 5 部：现代 OpenGL、工程化、调试与性能

| 课 | 内容 | 必须掌握或产出 |
|---|---|---|
| 5.1 | RAII 与 GL 对象生命周期 | 避免重复释放、拷贝持有和 Context 销毁顺序错误 |
| 5.2 | Shader/Texture/Mesh/Framebuffer 封装边界 | 封装资源但保留对底层状态的可见性 |
| 5.3 | Shader Hot Reload 与错误恢复 | 编译失败时保留可用旧 Program |
| 5.4 | Direct State Access（OpenGL 4.5+） | 对比 bind-to-edit 与 DSA，迁移部分资源代码 |
| 5.5 | Debug Output 与对象标签 | 按严重度过滤信息并定位出错调用 |
| 5.6 | RenderDoc 帧捕获 | 检查 Pipeline、Texture、Buffer、Draw Call 和 Shader |
| 5.7 | CPU 时间与 GPU Timer Query | 区分 CPU/GPU 瓶颈，避免只看 FPS |
| 5.8 | Draw Call、状态切换与批处理 | 用测量结果指导合批和减少冗余状态 |
| 5.9 | Buffer Streaming、映射与同步对象 | 理解 CPU/GPU Stall、Fence 和多缓冲策略 |
| 5.10 | Multi-draw 与 Indirect Drawing | 理解间接参数 Buffer，为 GPU Driven Rendering 做准备 |
| 5.11 | Frustum Culling、Occlusion 思路与 LOD | 建立可见性和细节层级的优化框架 |
| 5.12 | Texture Compression、Mipmap 与带宽 | 识别采样、显存和带宽开销 |
| 5.13 | 性能实验报告 | 固定场景、变量和采样方法，记录优化前后证据 |

**里程碑**：用 RenderDoc 和 Timer Query 分析自有场景，完成一项有数据支撑的优化。

## 第 6 部：Compute Shader 零基础入门

| 课 | 内容 | 必须掌握或产出 |
|---|---|---|
| 6.1 | Compute Shader 是什么、不是什么 | 理解它独立于光栅化图形管线，没有固定功能输出 |
| 6.2 | GPGPU、数据并行与适用任务 | 判断数组处理、粒子、图像、剔除等任务是否适合 Compute |
| 6.3 | OpenGL 4.3 要求与能力查询 | 检查版本及 Workgroup Count、Size、Shared Memory 限制 |
| 6.4 | 最小 Compute Program | 编译、链接仅含 Compute Shader 的 Program |
| 6.5 | Invocation 与 Workgroup | 区分单次调用、局部工作组和全局 Dispatch |
| 6.6 | `local_size_*` 与三维执行空间 | 理解 Shader 内局部大小和 CPU 端组数的关系 |
| 6.7 | 内置 ID | 使用 `gl_GlobalInvocationID`、`gl_WorkGroupID`、`gl_LocalInvocationID` 等定位数据 |
| 6.8 | `glDispatchCompute` 与向上取整 | 为一维/二维任务计算组数，并处理末尾越界 |
| 6.9 | 第一个 SSBO 数组程序 | 完成数组平方或向量加法，读回并与 CPU 结果逐项比较 |
| 6.10 | Compute 调试方法 | 从小组、小数据、唯一颜色和已知输入开始定位问题 |

**里程碑**：不依赖复制模板，独立完成 SSBO 向量加法；任意数据长度都不会漏算或越界。

## 第 7 部：Compute 数据资源与布局

| 课 | 内容 | 必须掌握或产出 |
|---|---|---|
| 7.1 | SSBO 创建、绑定与 Binding Point | 明确 Buffer、Binding 和 Shader Block 的连接方式 |
| 7.2 | UBO 与 SSBO 的选择 | 从大小、读写方式和用途比较二者 |
| 7.3 | `std430` 与 `std140` | 理解对齐、数组 Stride 和结构体布局 |
| 7.4 | `vec3`、Padding 与 CPU/GPU 结构匹配 | 能计算布局并用工具检查实际字节 |
| 7.5 | Texture Sampling 与 Image Load/Store | 区分规范化采样和整数 Texel 随机访问 |
| 7.6 | `imageLoad`、`imageStore` 与格式限定符 | 匹配纹理内部格式和 Shader Image Format |
| 7.7 | 第一张 Compute 输出图像 | 生成渐变或程序化纹理并显示到屏幕 |
| 7.8 | 只读、只写与访问限定符 | 根据真实访问方式选择 `readonly`、`writeonly`、`coherent` 等 |
| 7.9 | Atomic Counter 与统计输出 | 用最小示例理解共享计数需求 |
| 7.10 | 读回、Fence 与流水线 Stall | 区分调试验证和每帧同步读回的性能影响 |
| 7.11 | `glDispatchComputeIndirect` | 理解由 Buffer 提供 Dispatch 参数的用途 |

**里程碑**：Compute Shader 写入图像和 SSBO，CPU 验证结果，并能解释所有数据布局。

## 第 8 部：并行协作、同步与原子操作

| 课 | 内容 | 必须掌握或产出 |
|---|---|---|
| 8.1 | 独立 Workgroup 与未指定执行顺序 | 不依赖组间执行先后，识别数据竞争 |
| 8.2 | Shared Memory | 理解它只在同一 Workgroup 内共享及其容量限制 |
| 8.3 | `barrier()` 与 Uniform Control Flow | 保证组内所有 Invocation 到达同一 Barrier |
| 8.4 | 执行同步与内存可见性 | 区分 `barrier()`、`memoryBarrier*()` 和 `groupMemoryBarrier()` |
| 8.5 | CPU 端 `glMemoryBarrier` | 按后续消费者选择正确 Barrier Bit |
| 8.6 | Compute → SSBO/Texture/Vertex/Indirect 的同步 | 为不同消费者建立正确的命令间可见性 |
| 8.7 | Atomic 操作 | 使用 `atomicAdd`、Min/Max、Exchange、CompSwap 避免丢失更新 |
| 8.8 | Atomic 竞争与组内聚合 | 先在 Shared Memory 聚合，再减少全局 Atomic 次数 |
| 8.9 | Parallel Reduction | 用 Shared Memory 和 Barrier 实现 Sum/Min/Max |
| 8.10 | Histogram 与 Append Buffer | 实现直方图或用 Atomic 分配输出位置 |
| 8.11 | Prefix Sum（Scan）入门 | 理解 Scan 在 Compaction 和 GPU 算法中的作用 |
| 8.12 | 非确定性与正确性验证 | 区分顺序无关的合法结果和真正竞争错误 |

**里程碑**：完成 Shared Memory Reduction 和 Histogram，与 CPU 参考实现对照，并解释每处 Barrier。

## 第 9 部：Compute 性能、图形协作与综合项目

| 课 | 内容 | 必须掌握或产出 |
|---|---|---|
| 9.1 | Workgroup Size 实验 | 测量不同局部大小，不把固定经验值当作普遍最优 |
| 9.2 | Occupancy、寄存器与 Shared Memory 压力 | 理解资源使用为何限制并发并影响性能 |
| 9.3 | Coalescing、数据布局与带宽 | 比较连续/跨步访问及 AoS/SoA 实验结果 |
| 9.4 | Divergence 与算法冗余 | 识别分支、重复工作和全局同步成本 |
| 9.5 | Compute 图像后处理 | 与 Fragment Shader 版本比较功能和性能 |
| 9.6 | Compute Particle Simulation | 更新 SSBO 粒子数据，再交给 Vertex Shader 渲染 |
| 9.7 | GPU Frustum Culling + Indirect Draw | Compute 写可见列表和 Indirect 参数，随后绘制 |
| 9.8 | Compute 与 Fragment/Vertex 的任务选择 | 根据 Scatter、Shared Memory、数据流向选择实现 |
| 9.9 | Compute 常见故障清单 | 排查组数、越界、格式、布局、Barrier、竞争和同步读回 |
| 9.10 | 综合项目性能报告 | 用 Timer Query/RenderDoc 证明正确性和优化效果 |

**最终里程碑（二选一，另一个作为扩展）**：

1. **GPU 粒子系统**：Compute 负责生成、模拟和回收粒子，渲染管线读取同一 Buffer 完成实例绘制。
2. **GPU Driven 场景**：Compute 执行视锥剔除和可见列表压缩，写 Indirect Draw 参数，再由 OpenGL 绘制场景。

最终验收必须包含：CPU 小规模参考验证、正确 Barrier 说明、边界输入测试、RenderDoc 帧分析和优化前后性能数据。

## 第 10 部：扩展专题与能力边界

这些主题不阻塞主线毕业，可按项目需要选学：

- Transform Feedback 与 Compute Particle 方案比较。
- TAA、SSR、Volumetric Lighting、Terrain、Skeletal Animation。
- Forward+、Clustered Shading 与更深入的 GPU Driven Rendering。
- Shader Pipeline、Program Binary、跨 Context/多线程资源共享。
- 厂商扩展与跨厂商可移植性；扩展功能不能默认当作 Core OpenGL。
- OpenGL 与其他图形 API 的概念迁移；Vulkan/Direct3D 不属于本课程主线。
- Mesh Shader、硬件光线追踪不作为 Core OpenGL 必修内容。

## 主线完成检查表

- [ ] 能从空工程创建现代 OpenGL 渲染程序，而非只修改现成示例。
- [ ] 能解释 VAO/VBO/EBO、MVP、Framebuffer、Depth/Stencil 和纹理采样。
- [ ] 能实现并调试光照、阴影、后处理、Deferred Rendering 和 PBR/IBL。
- [ ] 能使用 RenderDoc、Debug Output 和 Timer Query 提供诊断证据。
- [ ] 能解释 Invocation、Workgroup、Dispatch 和所有内置 ID。
- [ ] 能正确使用 SSBO、Image、`std430` 和 CPU/GPU 数据布局。
- [ ] 能区分组内执行 Barrier、Shader 内存 Barrier 与 CPU 端 `glMemoryBarrier`。
- [ ] 能实现 Reduction、Histogram 或 Scan，并处理竞争与边界。
- [ ] 能完成 Compute → 渲染或 Compute → Indirect Draw 的综合项目。
