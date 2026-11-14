cmake_minimum_required(VERSION 3.28)
project(libraries-demo LANGUAGES CXX)

# INTERFACE 库没有编译产物，只描述「消费我需要什么」
add_library(veclib INTERFACE)
target_include_directories(veclib INTERFACE include)
# 语义要求传播给下游：任何链接 veclib 的目标自动获得 C++17
target_compile_features(veclib INTERFACE cxx_std_17)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE veclib)
