cmake_minimum_required(VERSION 3.21)
project(part02_01_lighting_basics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(part02_01_lighting_basics main.cpp)
target_compile_definitions(part02_01_lighting_basics PRIVATE GLFW_INCLUDE_NONE)
target_include_directories(part02_01_lighting_basics PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../part-01-common")
target_link_libraries(part02_01_lighting_basics PRIVATE glfw glad::glad glm::glm OpenGL::GL)

if(MSVC)
    target_compile_options(part02_01_lighting_basics PRIVATE /utf-8)
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(NAME part02_01_lighting_basics_smoke COMMAND part02_01_lighting_basics --hidden --max-frames 2)
endif()
