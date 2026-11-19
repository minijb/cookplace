// 06 类型双关与严格别名：如何合法地「重新解释」字节
// C++17；构建验证：Windows + MSVC 19.50（见 README）
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <type_traits>

// 把任意平凡可复制对象的字节安全地复制成另一个类型（C++17 正解）
template <class Dst, class Src>
Dst bit_cast_copy(const Src& src) {
    static_assert(sizeof(Dst) == sizeof(Src), "bit 级转换要求大小一致");
    static_assert(std::is_trivially_copyable<Src>::value, "Src 必须平凡可复制");
    static_assert(std::is_trivially_copyable<Dst>::value, "Dst 必须平凡可复制");
    Dst dst;
    std::memcpy(&dst, &src, sizeof(Dst));   // 按字节复制：唯一标准认可的双关方式
    return dst;
}

int main() {
    // ---- 非法姿势（本文件不执行，只列出来）：----
    // float f = 1.0f;
    // int bits_bad = *reinterpret_cast<int*>(&f);   // UB：违反严格别名规则
    // union { float f; int i; } u; u.f = f;
    // int bits_union = u.i;                          // C++ 中读非活动成员是 UB

    // ---- 合法姿势 1：memcpy 按字节复制 ----
    float f = 1.0f;
    std::uint32_t bits = bit_cast_copy<std::uint32_t>(f);
    std::printf("float %.4f 的位模式 = 0x%08X（IEEE 754 符号 0，指数 127，尾数全 0）\n",
                f, bits);
    float round_trip = bit_cast_copy<float>(bits);
    std::printf("位模式复制回 float = %.4f（无损往返）\n", round_trip);

    // ---- 合法姿势 2：char/unsigned char/std::byte 指针可以为任何对象别名 ----
    std::uint32_t value = 0x12345678;
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&value);
    std::printf("按字节查看 uint32_t：%02X %02X %02X %02X（x86 小端序：低字节在前）\n",
                bytes[0], bytes[1], bytes[2], bytes[3]);

    // ---- 位级操作的对象必须是平凡可复制；带虚表/引用成员的不行 ----
    struct Config {                       // 平凡可复制：可以直接按字节双关
        std::uint16_t width;
        std::uint16_t height;
        float scale;
    };
    struct WireFormat {                   // 网络字节序视图（假设），同样大小
        std::uint8_t raw[8];
    };
    static_assert(sizeof(Config) == sizeof(WireFormat), "大小必须一致");
    static_assert(std::is_trivially_copyable<Config>::value, "POD 配置可双关");

    Config cfg{1920, 1080, 1.5f};
    WireFormat wire = bit_cast_copy<WireFormat>(cfg);
    Config restored = bit_cast_copy<Config>(wire);
    std::printf("Config {1920,1080,1.5} 字节化再还原：{%u,%u,%.1f}\n",
                restored.width, restored.height, restored.scale);

    // ---- 用 static_assert 提前拦住大小不匹配 ----
    // bit_cast_copy<std::uint64_t>(f);   // static_assert 失败：编译错误，不是运行期错误

    // ---- 展望 C++20：std::bit_cast 取代手写 memcpy 助手 ----
    // #include <bit>
    // auto bits20 = std::bit_cast<std::uint32_t>(f);   // constexpr，编译期可用
    std::printf("C++17 用 memcpy 助手；C++20 起 std::bit_cast 是标准答案\n");
    return 0;
}
