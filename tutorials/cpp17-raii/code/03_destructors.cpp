#include <cassert>
#include <iostream>
#include <type_traits>

class DeviceHandle {
public:
    explicit DeviceHandle(int id) : id_(id) {
        ++open_count_;
    }

    ~DeviceHandle() noexcept {
        static_cast<void>(close());
    }

    DeviceHandle(const DeviceHandle&) = delete;
    DeviceHandle& operator=(const DeviceHandle&) = delete;

    bool close() noexcept {
        if (id_ < 0) {
            return true;
        }
        id_ = -1;
        --open_count_;
        return true;
    }

    bool is_open() const {
        return id_ >= 0;
    }

    static int open_count() {
        return open_count_;
    }

private:
    int id_;
    inline static int open_count_ = 0;
};

static_assert(std::is_nothrow_destructible_v<DeviceHandle>);

int main() {
    DeviceHandle device(7);
    assert(DeviceHandle::open_count() == 1);

    const bool closed = device.close();
    assert(closed);
    assert(!device.is_open());
    assert(DeviceHandle::open_count() == 0);

    const bool closed_again = device.close();
    assert(closed_again);

    {
        DeviceHandle automatic_cleanup(9);
        assert(DeviceHandle::open_count() == 1);
    }
    assert(DeviceHandle::open_count() == 0);

    std::cout << "explicit close reports success; destructor is noexcept\n";
}
