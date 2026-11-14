# 示例代码索引

每个目录对应一课，均可独立构建。全部示例已于 2026-11-14 在 Windows 11 + MSVC 14.39 + CMake 3.29.2 + vcpkg 2026-10-29 环境构建并运行通过。

| 目录 | 对应课 | 演示点 | 构建方式 |
|---|---|---|---|
| [part-01-03-hello](part-01-03-hello/) | part-01/03 | 最小 CMake 工程、三阶段模型 | `cmake -S . -B build && cmake --build build` |
| [part-01-04-targets](part-01-04-targets/) | part-01/04 | 静态库 + 传播关键字 | 同上 |
| [part-01-05-libraries](part-01-05-libraries/) | part-01/05 | INTERFACE header-only 库 | 同上 |
| [part-01-07-deps](part-01-07-deps/) | part-01/07 | FetchContent 拉 fmt | 同上（首次需联网克隆 fmt） |
| [part-01-08-presets](part-01-08-presets/) | part-01/08 | presets + CTest | `cmake --preset msvc-debug && cmake --build --preset msvc-debug && ctest --preset msvc-debug` |
| [part-03-01-fmt-vcpkg](part-03-01-fmt-vcpkg/) | part-03/01 | vcpkg toolchain 接线 + DLL 自动部署 | 同上 preset 三连 |
| [part-03-02-opengl](part-03-02-opengl/) | part-03/02 | 毕业项目：GLFW + glad 三角形 | 同上 preset 三连，运行 `build\opengl-hello.exe` |

## 运行前提

- Ninja 预设（08 及以后）需要 MSVC 环境：VSCode 里 CMake Tools 自动处理；命令行先跑 `vcvars64.bat`（路径见 part-01/01）
- vcpkg 示例（03-01、03-02）需要 vcpkg 安装在 `C:\dev\vcpkg`（part-01/01），或改 preset 里的 `CMAKE_TOOLCHAIN_FILE` 路径
- 构建产物 `build/`、`vcpkg_installed/` 均不入库，删掉即可完整重置
