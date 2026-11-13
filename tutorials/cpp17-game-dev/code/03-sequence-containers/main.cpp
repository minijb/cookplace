#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <deque>
#include <forward_list>
#include <iostream>
#include <iterator>
#include <list>
#include <vector>

namespace {

struct CopyPreferred {
    static inline std::size_t copies = 0;
    static inline std::size_t moves = 0;

    int value = 0;

    CopyPreferred() = default;
    explicit CopyPreferred(int value) : value(value) {}
    CopyPreferred(const CopyPreferred& other) noexcept : value(other.value) {
        ++copies;
    }
    CopyPreferred(CopyPreferred&& other) noexcept(false) : value(other.value) {
        ++moves;
        other.value = -1;
    }
    CopyPreferred& operator=(const CopyPreferred&) = default;
    CopyPreferred& operator=(CopyPreferred&&) = default;
};

struct MovePreferred {
    static inline std::size_t copies = 0;
    static inline std::size_t moves = 0;

    int value = 0;

    MovePreferred() = default;
    explicit MovePreferred(int value) : value(value) {}
    MovePreferred(const MovePreferred& other) noexcept : value(other.value) {
        ++copies;
    }
    MovePreferred(MovePreferred&& other) noexcept : value(other.value) {
        ++moves;
        other.value = -1;
    }
    MovePreferred& operator=(const MovePreferred&) = default;
    MovePreferred& operator=(MovePreferred&&) = default;
};

void demo_array_and_vector() {
    std::array<int, 3> fixed{{1, 2, 3}};
    assert(fixed.size() == 3);

    std::vector<int> values;
    values.reserve(3);
    values.insert(values.end(), {10, 20, 30});
    while (values.size() < values.capacity()) {
        values.push_back(30);
    }
    assert(values.data()[1] == values[1]);

    const auto old_size = values.size();
    const auto old_capacity = values.capacity();
    const int* old_data = values.data();
    values.push_back(40);
    assert(values.size() == old_size + 1);
    assert(values.capacity() > old_capacity);
    assert(values.data() != old_data);
    // old_data 已失效，不能再解引用。

    values.reserve(128);
    assert(values.size() == old_size + 1 && values.capacity() >= 128);
    values.shrink_to_fit();
    assert(values.size() == old_size + 1);
}

void demo_deque() {
    std::deque<int> values{2, 3};
    const int& two = values.front();
    const int& three = values.back();

    values.push_front(1);
    values.push_back(4);

    // 两端插入会使迭代器失效，但不使其他元素的引用失效。
    assert(two == 2);
    assert(three == 3);
    assert(values.front() == 1 && values.back() == 4);
}

void demo_node_sequences() {
    std::list<int> values{1, 2, 3};
    const auto second = std::next(values.begin());
    const int* second_address = &*second;

    values.push_front(0);
    values.erase(values.begin());

    assert(*second == 2);
    assert(second_address == &*second);

    std::forward_list<int> forward_values{2, 3};
    forward_values.insert_after(forward_values.before_begin(), 1);
    assert(forward_values.front() == 1);
}

void demo_exception_safe_growth() {
    std::vector<CopyPreferred> copying;
    copying.reserve(1);
    CopyPreferred first_copy{1};
    copying.push_back(first_copy);
    while (copying.size() < copying.capacity()) {
        copying.push_back(first_copy);
    }
    const auto copying_old_size = copying.size();
    CopyPreferred::copies = 0;
    CopyPreferred::moves = 0;

    CopyPreferred second_copy{2};
    copying.push_back(second_copy);
    assert(CopyPreferred::copies == copying_old_size + 1);
    assert(CopyPreferred::moves == 0);

    std::vector<MovePreferred> moving;
    moving.reserve(1);
    MovePreferred first_move{1};
    moving.push_back(first_move);
    while (moving.size() < moving.capacity()) {
        moving.push_back(first_move);
    }
    const auto moving_old_size = moving.size();
    MovePreferred::copies = 0;
    MovePreferred::moves = 0;

    MovePreferred second_move{2};
    moving.push_back(second_move);
    assert(MovePreferred::copies == 1);
    assert(MovePreferred::moves == moving_old_size);
}

} // namespace

int main() {
    demo_array_and_vector();
    demo_deque();
    demo_node_sequences();
    demo_exception_safe_growth();
    std::cout << "sequence container checks passed\n";
}
