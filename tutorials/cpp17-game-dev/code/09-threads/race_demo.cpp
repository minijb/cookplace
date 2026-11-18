// 独立的数据竞争演示程序：供 TSan 观察报告使用，故意不注册进 CTest。
//
//   GCC/Clang:  g++ -g -O1 -fsanitize=thread race_demo.cpp -o race_demo && ./race_demo
//   MSVC:       cl /Zi /fsanitize=thread race_demo.cpp（VS 2022 起实验特性，x64）
//
// 两个线程无同步地写同一个 int，这是数据竞争（UB）。TSan 会报告
// 「data race on unsynchronized access」并给出两条冲突访问的堆栈。

#include <iostream>
#include <thread>
#include <vector>

int main() {
    int unsafe = 0;

    std::vector<std::thread> workers;
    for (int t = 0; t < 4; ++t) {
        workers.emplace_back([&unsafe] {
            for (int i = 0; i < 10000; ++i) {
                ++unsafe; // 数据竞争：++unsafe 是 load + store 两步
            }
        });
    }
    for (auto& worker : workers) {
        worker.join();
    }

    std::cout << "expected 40000, got " << unsafe << '\n';
    return 0;
}
