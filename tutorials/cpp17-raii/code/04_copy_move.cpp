#include <cassert>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

class Handle {
public:
    explicit Handle(int id) : id_(id) {
        if (id_ < 0) {
            throw std::invalid_argument("handle id must be non-negative");
        }
        ++live_count_;
    }

    ~Handle() {
        release();
    }

    Handle(const Handle& other) : id_(other.id_) {
        if (id_ >= 0) {
            ++live_count_;
        }
    }

    Handle& operator=(const Handle& other) {
        Handle copy(other);
        swap(copy);
        return *this;
    }

    Handle(Handle&& other) noexcept : id_(std::exchange(other.id_, -1)) {}

    Handle& operator=(Handle&& other) noexcept {
        if (this != &other) {
            release();
            id_ = std::exchange(other.id_, -1);
        }
        return *this;
    }

    void swap(Handle& other) noexcept {
        std::swap(id_, other.id_);
    }

    int id() const {
        return id_;
    }

    static int live_count() {
        return live_count_;
    }

private:
    void release() noexcept {
        if (id_ >= 0) {
            --live_count_;
            id_ = -1;
        }
    }

    int id_;
    inline static int live_count_ = 0;
};

static_assert(std::is_nothrow_move_constructible_v<Handle>);
static_assert(std::is_nothrow_move_assignable_v<Handle>);

int main() {
    Handle original(10);
    {
        Handle copy(original);
        assert(Handle::live_count() == 2);
        assert(copy.id() == original.id());

        Handle moved(std::move(copy));
        assert(copy.id() == -1);
        assert(moved.id() == 10);
        assert(Handle::live_count() == 2);

        Handle assigned(20);
        assigned = std::move(moved);
        assert(assigned.id() == 10);
        assert(moved.id() == -1);
        assert(Handle::live_count() == 2);
    }

    assert(Handle::live_count() == 1);
    std::cout << "copy duplicates ownership; move transfers ownership\n";
}
