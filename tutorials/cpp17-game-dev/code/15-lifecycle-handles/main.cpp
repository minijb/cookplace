#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#define REQUIRE(cond)                                                          \
    do {                                                                       \
        if (!(cond)) {                                                         \
            std::cerr << "REQUIRE failed: " #cond " at line " << __LINE__      \
                      << '\n';                                                 \
            std::abort();                                                      \
        }                                                                      \
    } while (0)

// ---------------------------------------------------------------------------
// 1. 句柄表：索引 + 代数，跨帧安全地引用资源
// ---------------------------------------------------------------------------

struct Texture {
    int width = 0;
    int height = 0;
    explicit Texture(int w, int h) : width(w), height(h) {}
};

struct TextureHandle {
    std::uint32_t index = ~std::uint32_t{0};
    std::uint32_t generation = 0;
};

class TextureTable {
public:
    TextureHandle load(int width, int height) {
        std::uint32_t index;
        if (!free_.empty()) {
            index = free_.back();
            free_.pop_back();
            textures_[index] = std::make_unique<Texture>(width, height);
        } else {
            index = static_cast<std::uint32_t>(textures_.size());
            textures_.push_back(std::make_unique<Texture>(width, height));
            generations_.push_back(0);
        }
        return TextureHandle{index, generations_[index]};
    }

    Texture* resolve(TextureHandle handle) {
        if (handle.index >= textures_.size()) {
            return nullptr;
        }
        if (generations_[handle.index] != handle.generation) {
            return nullptr; // 代数不符：句柄指向的槽位已被释放/复用
        }
        return textures_[handle.index].get();
    }

    void unload(TextureHandle handle) {
        if (resolve(handle) == nullptr) {
            return; // 已失效，重复 unload 无害
        }
        textures_[handle.index].reset();
        ++generations_[handle.index]; // 递增代数：所有旧句柄即刻失效
        free_.push_back(handle.index);
    }

private:
    std::vector<std::unique_ptr<Texture>> textures_;
    std::vector<std::uint32_t> generations_;
    std::vector<std::uint32_t> free_;
};

void handle_demo() {
    TextureTable table;
    const TextureHandle hero = table.load(64, 64);

    Texture* resolved = table.resolve(hero);
    REQUIRE(resolved != nullptr && resolved->width == 64);

    table.unload(hero);
    REQUIRE(table.resolve(hero) == nullptr); // 悬垂句柄被检测，而不是误读

    // 槽位复用：新资源拿同一 index、递增后的 generation。
    const TextureHandle slime = table.load(32, 32);
    REQUIRE(slime.index == hero.index);
    REQUIRE(slime.generation == hero.generation + 1);
    REQUIRE(table.resolve(hero) == nullptr);       // 旧句柄仍失效
    REQUIRE(table.resolve(slime)->width == 32);    // 新句柄正常

    table.unload(hero); // 重复 unload：resolve 已判失效，安全无副作用
}

// ---------------------------------------------------------------------------
// 2. 帧分配器：整帧重置的线性 arena
// ---------------------------------------------------------------------------

class FrameAllocator {
public:
    explicit FrameAllocator(std::size_t bytes) : buffer_(bytes) {}

    template <typename T, typename... Args>
    T* create(Args&&... args) {
        std::size_t space = buffer_.size() - offset_; // std::align 要求可写引用
        const std::size_t alignment = alignof(T);
        void* start = buffer_.data() + offset_;
        // std::align：在剩余空间里对齐，返回可用地址并前移 offset_。
        void* aligned = std::align(alignment, sizeof(T), start, space);
        if (aligned == nullptr) {
            return nullptr; // 帧内存耗尽：调用方决定策略
        }
        offset_ = static_cast<std::size_t>(static_cast<char*>(aligned) -
                                           buffer_.data()) +
                  sizeof(T);
        return new (aligned) T(std::forward<Args>(args)...); // placement new
    }

    template <typename T>
    void destroy(T* object) {
        object->~T(); // 显式调析构；内存不还给堆，offset 稍后统一回卷
    }

    void reset() { offset_ = 0; } // 帧末一次性重置，O(1)
    std::size_t used() const { return offset_; }
    std::size_t capacity() const { return buffer_.size(); }

private:
    std::vector<char> buffer_;
    std::size_t offset_ = 0;
};

struct FrameVector {
    explicit FrameVector(std::size_t n) {
        data.resize(n);
        for (std::size_t i = 0; i < n; ++i) {
            data[static_cast<std::size_t>(i)] = static_cast<int>(i);
        }
    }
    std::vector<int> data; // 借堆；真实帧分配器会连 vector 一起帧内分配
};

void frame_allocator_demo() {
    FrameAllocator frame(4096);

    FrameVector* a = frame.create<FrameVector>(3);
    REQUIRE(a != nullptr && a->data.size() == 3);
    REQUIRE(frame.used() > 0);

    // 帧末：先析构有生命周期的对象，再 O(1) 回卷整段内存。
    frame.destroy(a);
    frame.reset();
    REQUIRE(frame.used() == 0);

    // 下一帧：同一块内存被新对象复用，没有任何堆分配。
    FrameVector* b = frame.create<FrameVector>(3);
    REQUIRE(b != nullptr && b->data[2] == 2);
    REQUIRE(frame.used() > 0);
    frame.destroy(b);
    frame.reset();
}

// ---------------------------------------------------------------------------
// 3. 关卡生命周期：RAII 阶段与加载/卸载顺序
// ---------------------------------------------------------------------------

class Level {
public:
    Level(std::vector<std::string>& log, TextureTable& table)
        : log_(log), table_(table) {
        // 加载顺序：先底层资源，后上层对象——卸载按相反顺序进行。
        hero_ = table_.load(64, 64);
        log_.push_back("resources-loaded");
        log_.push_back("objects-spawned");
        ready_ = true;
    }

    ~Level() {
        // 卸载顺序与加载相反：先拆对象引用，再释放资源。
        log_.push_back("objects-destroyed");
        table_.unload(hero_);
        log_.push_back("resources-unloaded");
    }

    bool ready() const { return ready_; }
    TextureHandle hero_handle() const { return hero_; }

private:
    std::vector<std::string>& log_;
    TextureTable& table_;
    TextureHandle hero_;
    bool ready_ = false;
};

void level_lifecycle_demo() {
    TextureTable table;
    std::vector<std::string> log;

    // 外部早已加载的资源：不随关卡卸载而消失。
    const TextureHandle ui_icon = table.load(16, 16);
    {
        Level level(log, table);
        REQUIRE(level.ready());
        // 裸指针可以指进关卡，但关卡卸载后就是悬垂——句柄不会。
        REQUIRE(table.resolve(level.hero_handle()) != nullptr);
    } // RAII：作用域结束即卸载，顺序确定

    const char* expected[] = {
        "resources-loaded", "objects-spawned", "objects-destroyed",
        "resources-unloaded"};
    REQUIRE(log.size() == 4);
    for (std::size_t i = 0; i < 4; ++i) {
        REQUIRE(log[i] == expected[i]);
    }
    REQUIRE(table.resolve(ui_icon) != nullptr); // 外部资源完好
}

int main() {
    handle_demo();
    frame_allocator_demo();
    level_lifecycle_demo();
    std::cout << "ch04_lifecycle_handles: all checks passed\n";
    return 0;
}
