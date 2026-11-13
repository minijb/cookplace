# pitfall：无 BOM 中文注释被 MSVC 按 GBK 误解码，导致断言错乱

环境：Windows 11 · MSVC 19.50（cl.exe，代码页 936） · CMake 4.2.1 · 2026-11-13

现象：含中文注释的 UTF-8（无 BOM）源文件编译时报警告 `C4819: 该文件包含不能在当前代码页(936)中表示的字符`；`ch02_entity_storage` 测试中与逻辑推演矛盾的断言失败（`store.size() == 1` 失败，静态推演应为通过），失败测试在 CTest 下阻塞 143 秒。多数含中文注释的目标仍能通过，仅个别文件出错。

原因：MSVC 无 BOM 时按系统代码页（中文 Windows 为 GBK/CP936）解码源文件。UTF-8 中文注释的字节流被误解码为 GBK 字符对时，可能吞掉行尾或后续 ASCII 字节，改变源码的实际 token 流；损坏位置取决于具体字节序列，因此只有部分文件出问题。逻辑推演基于正确源码，与编译器实际看到的源码不一致。

解法：

1. 为含非 ASCII 字符的源文件添加 UTF-8 BOM（`EF BB BF`），MSVC 检测到 BOM 后按 UTF-8 解码。
2. 或将源码中的注释改为纯 ASCII（英文），本仓库第一章源码即采用此方式。
3. 或在 CMake 中为 MSVC 显式指定源码编码：`add_compile_options(/utf-8)`（同时影响执行字符集，需整体评估）。
4. 修复后重新完整构建并跑 CTest 确认；不要依据"大部分目标能编译"判断源码未被损坏。

相关：[C++17 游戏开发进阶·第二章](../../tutorials/cpp17-game-dev/part-02-stl/04-game-entity-storage/index.md)；[验证记录](../../tutorials/cpp17-game-dev/resources/ch02-research.md)
