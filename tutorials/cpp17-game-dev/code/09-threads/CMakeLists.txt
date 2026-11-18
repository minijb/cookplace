add_executable(ch03_threads main.cpp)
add_test(NAME ch03_threads COMMAND ch03_threads)

# 数据竞争演示程序：仅供 TSan 手动运行，不注册进 CTest。
add_executable(ch03_race_demo race_demo.cpp)
