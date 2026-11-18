cmake_minimum_required(VERSION 3.21)
project(part01_13_depth_culling LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)

add_executable(part01_13_depth_culling main.cpp)
target_compile_definitions(part01_13_depth_culling PRIVATE GLFW_INCLUDE_NONE)
target_compile_definitions(part01_13_depth_culling PRIVATE PART01_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../part-01-assets")
target_include_directories(part01_13_depth_culling PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../part-01-common")
target_include_directories(part01_13_depth_culling PRIVATE ${Stb_INCLUDE_DIR})
target_link_libraries(part01_13_depth_culling PRIVATE glfw glad::glad glm::glm OpenGL::GL)

if(MSVC)
    target_compile_options(part01_13_depth_culling PRIVATE /utf-8)
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(NAME part01_13_depth_culling_smoke COMMAND part01_13_depth_culling --hidden --max-frames 2)
endif()
