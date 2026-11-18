#include <cassert>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// 每个检查都真正执行（不受 NDEBUG 影响），保证 Release 下 CTest 仍然校验行为。
#define REQUIRE(cond)                                                          \
    do {                                                                       \
        if (!(cond)) {                                                         \
            std::cerr << "REQUIRE failed: " #cond " at line " << __LINE__      \
                      << '\n';                                                 \
            std::abort();                                                      \
        }                                                                      \
    } while (0)

// ---------------------------------------------------------------------------
// 1. 函数模板与实参推导：按值传递会 decay，按引用传递保留引用性
// ---------------------------------------------------------------------------

template <typename T>
T sum_by_value(T a, T b) {
    return a + b;
}

template <typename T>
constexpr T max_by_ref(const T& a, const T& b) {
    return a < b ? b : a;
}

void deduction_basics() {
    REQUIRE(sum_by_value(2, 3) == 5);       // T 推导为 int
    REQUIRE(sum_by_value(2.0, 3.5) == 5.5); // T 推导为 double

    const int big = 10;
    const int small = 3;
    // 按值传参：T 推导为 int，顶屋 const 与数组/函数到指针的 decay 都会发生。
    REQUIRE(sum_by_value(big, small) == 13);
    // 按 const T& 传参：T 推导为 int，const 保留在参数类型里。
    REQUIRE(max_by_ref(big, small) == 10);
    static_assert(std::is_same<decltype(max_by_ref(big, small)), int>::value,
                  "返回类型跟随 T 的推导结果");
}

// ---------------------------------------------------------------------------
// 2. 类模板、CTAD 与推导指引
// ---------------------------------------------------------------------------

template <typename T>
class ResourceCache {
public:
    ResourceCache() = default;
    ResourceCache(std::initializer_list<T> items) {
        for (const T& item : items) {
            put(item);
        }
    }
    // 借助 CTAD，从「以 T 为元素的可迭代范围」推导 ResourceCache<T>。
    ResourceCache(const std::vector<T>& items) {
        for (const T& item : items) {
            put(item);
        }
    }

    void put(const T& item) {
        if (std::find(items_.begin(), items_.end(), item) == items_.end()) {
            items_.push_back(item);
        }
    }
    std::size_t size() const { return items_.size(); }
    bool has(const T& item) const {
        return std::find(items_.begin(), items_.end(), item) != items_.end();
    }

private:
    std::vector<T> items_;
};

// C++17 类模板实参推导（CTAD）：无需写 ResourceCache<const char*>。
void ctad_basics() {
    ResourceCache names{std::string{"hero"}, std::string{"slime"},
                        std::string{"hero"}}; // 推导为 ResourceCache<std::string>
    REQUIRE(names.size() == 2);               // 重复的 "hero" 只保留一份
    REQUIRE(names.has(std::string{"slime"}));

    std::vector<int> ids{1, 2, 3};
    // 注意：用花括号 ResourceCache cached{ids} 会优先匹配 initializer_list
    // 构造函数，把 ids 当成单个元素（得到 vector<int> 缓存）。要从可迭代范
    // 围推导，用括号初始化走 vector 构造函数。
    ResourceCache cached(ids); // 从 vector<int> 推导为 ResourceCache<int>
    REQUIRE(cached.size() == 3);
}

// ---------------------------------------------------------------------------
// 3. 非类型模板参数：把编译期常量编进类型
// ---------------------------------------------------------------------------

template <typename T, std::size_t N>
class RingBuffer {
public:
    bool push(const T& value) {
        if (count_ == N) {
            return false; // 教学版：满则拒绝，不做覆盖策略
        }
        slots_[tail_] = value;
        tail_ = (tail_ + 1) % N;
        ++count_;
        return true;
    }
    bool pop(T& out) {
        if (count_ == 0) {
            return false;
        }
        out = slots_[head_];
        head_ = (head_ + 1) % N;
        --count_;
        return true;
    }
    std::size_t size() const { return count_; }
    static constexpr std::size_t capacity() { return N; }

private:
    T slots_[N]{};
    std::size_t head_ = 0;
    std::size_t tail_ = 0;
    std::size_t count_ = 0;
};

void nontype_parameter() {
    RingBuffer<int, 4> events; // 容量 4 编译期已知，可与容量 8 的实例区分类型
    for (int i = 1; i <= 5; ++i) {
        const bool ok = events.push(i);
        REQUIRE(ok == (i <= 4)); // 第 5 个推入失败
    }
    REQUIRE(events.size() == 4);
    REQUIRE(events.capacity() == 4);

    int out = 0;
    REQUIRE(events.pop(out) && out == 1);
    REQUIRE(events.push(5)); // 弹出一个后又有空位
    REQUIRE(events.size() == 4);
}

// ---------------------------------------------------------------------------
// 4. 函数模板重载与特化的取舍：优先重载，少用特化
// ---------------------------------------------------------------------------

template <typename T>
const char* describe(const T&) {
    return "generic";
}
// 普通函数重载：完全匹配时优先于模板实例。
const char* describe(const std::string&) {
    return "string";
}
// 另一个模板重载：更特化的模板在重载决议中胜出。
template <typename T>
const char* describe(const std::vector<T>&) {
    return "vector";
}

void overloading() {
    REQUIRE(std::string{describe(42)} == "generic");
    REQUIRE(std::string{describe(std::string{"hi"})} == "string");
    REQUIRE(std::string{describe(std::vector<int>{1})} == "vector");
}

int main() {
    deduction_basics();
    ctad_basics();
    nontype_parameter();
    overloading();
    std::cout << "ch03_templates: all checks passed\n";
    return 0;
}
