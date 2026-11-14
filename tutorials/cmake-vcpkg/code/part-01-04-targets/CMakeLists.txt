cmake_minimum_required(VERSION 3.28)
project(targets-demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 静态库：编译产物是 math.lib，供下游链接
add_library(math STATIC src/math.cpp)
# PUBLIC：include 目录既用于编译 math 自身，也传播给链接 math 的下游
target_include_directories(math PUBLIC include)

add_executable(app main.cpp)
# PRIVATE：app 消费 math 的接口，但 app 不需要再把 math 传给更下游
target_link_libraries(app PRIVATE math)
