add_executable(ch04_ecs main.cpp)
add_test(NAME ch04_ecs COMMAND ch04_ecs)

# EnTT 对照示例：使用 vendored 头文件（third_party/entt），无网络依赖。
add_executable(ch04_entt_demo entt_demo.cpp)
target_include_directories(ch04_entt_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../third_party)
add_test(NAME ch04_entt_demo COMMAND ch04_entt_demo)
