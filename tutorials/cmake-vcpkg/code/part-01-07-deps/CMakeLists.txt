cmake_minimum_required(VERSION 3.28)
project(deps-demo LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        11.1.4   # 永远固定 tag，不用可变的分支名
)
FetchContent_MakeAvailable(fmt)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE fmt::fmt)
