cmake_minimum_required(VERSION 3.16)

project(cpp17_game_dev_tutorial LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive-)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_subdirectory(01-modern-syntax-and-raii)
add_subdirectory(02-move-semantics-and-smart-pointers)
add_subdirectory(03-sequence-containers)
add_subdirectory(04-associative-containers)
add_subdirectory(05-iterators-algorithms)
add_subdirectory(06-game-entity-storage)
