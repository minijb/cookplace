cmake_minimum_required(VERSION 3.21)
project(opengl_part00_ch07 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_executable(ch07_debug main.cpp)
target_compile_definitions(ch07_debug PRIVATE GLFW_INCLUDE_NONE)
target_link_libraries(ch07_debug PRIVATE glfw glad::glad OpenGL::GL)

if(MSVC)
    target_compile_options(ch07_debug PRIVATE /utf-8)
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(NAME ch07_debug_smoke COMMAND ch07_debug --hidden --max-frames 2)
    add_test(NAME ch07_debug_error COMMAND ch07_debug --hidden --max-frames 1 --trigger-error)
    set_tests_properties(ch07_debug_smoke PROPERTIES
        FAIL_REGULAR_EXPRESSION "\\| Error \\|")
    set_tests_properties(ch07_debug_error PROPERTIES
        PASS_REGULAR_EXPRESSION "GL_INVALID_ENUM"
        FAIL_REGULAR_EXPRESSION "GL_INVALID_VALUE")
endif()
