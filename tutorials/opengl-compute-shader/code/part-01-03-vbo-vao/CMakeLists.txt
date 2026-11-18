cmake_minimum_required(VERSION 3.21)
project(part01_03_vbo_vao LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(part01_03_vbo_vao main.cpp)
target_compile_definitions(part01_03_vbo_vao PRIVATE GLFW_INCLUDE_NONE)
target_include_directories(part01_03_vbo_vao PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../part-01-common")
target_link_libraries(part01_03_vbo_vao PRIVATE glfw glad::glad glm::glm OpenGL::GL)

if(MSVC)
    target_compile_options(part01_03_vbo_vao PRIVATE /utf-8)
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(NAME part01_03_vbo_vao_smoke COMMAND part01_03_vbo_vao --hidden --max-frames 2)
endif()
