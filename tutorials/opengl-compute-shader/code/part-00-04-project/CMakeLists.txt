cmake_minimum_required(VERSION 3.21)
project(opengl_part00_ch04 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_executable(ch04_project src/main.cpp)
target_compile_definitions(ch04_project PRIVATE GLFW_INCLUDE_NONE)
target_link_libraries(ch04_project PRIVATE glfw glad::glad OpenGL::GL)

if(MSVC)
    target_compile_options(ch04_project PRIVATE /utf-8)
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(NAME ch04_project_smoke COMMAND ch04_project --hidden --max-frames 2)
endif()
