cmake_minimum_required(VERSION 3.20)
project(opengl-hello LANGUAGES CXX)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_executable(opengl-hello main.cpp)
target_link_libraries(opengl-hello PRIVATE glfw glad::glad)

# MSVC 默认按系统代码页（GBK）读源文件，含中文注释会触发 C4819；
# /utf-8 让源码与执行字符集都按 UTF-8 处理
if(MSVC)
    target_compile_options(opengl-hello PRIVATE /utf-8)
endif()
