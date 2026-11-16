cmake_minimum_required(VERSION 3.20)
project(opengl-hello LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)        # 系统组件（opengl32）：CMake 内置 FindOpenGL，不经 vcpkg
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_executable(opengl-hello main.cpp)

target_compile_definitions(opengl-hello PRIVATE GLFW_INCLUDE_NONE)
target_link_libraries(opengl-hello PRIVATE glfw glm::glm glad::glad OpenGL::GL)

# MSVC 默认按系统代码页（GBK）读源文件，含中文注释会触发 C4819；
# /utf-8 让源码与执行字符集都按 UTF-8 处理
if(MSVC)
    target_compile_options(opengl-hello PRIVATE /utf-8)
endif()
