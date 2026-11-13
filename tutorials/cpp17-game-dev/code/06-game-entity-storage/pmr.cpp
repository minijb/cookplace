#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory_resource>
#include <numeric>
#include <vector>

namespace {

class CountingResource final : public std::pmr::memory_resource {
public:
    explicit CountingResource(
        std::pmr::memory_resource* upstream = std::pmr::get_default_resource())
        : upstream_(upstream) {}

    std::size_t allocations() const {
        return allocations_;
    }

    std::size_t deallocations() const {
        return deallocations_;
    }

private:
    void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        ++allocations_;
        return upstream_->allocate(bytes, alignment);
    }

    void do_deallocate(void* pointer, std::size_t bytes, std::size_t alignment) override {
        ++deallocations_;
        upstream_->deallocate(pointer, bytes, alignment);
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }

    std::pmr::memory_resource* upstream_;
    std::size_t allocations_ = 0;
    std::size_t deallocations_ = 0;
};

} // namespace

int main() {
    CountingResource upstream;
    {
        alignas(std::max_align_t) std::array<std::byte, 64> local_buffer{};
        std::pmr::monotonic_buffer_resource arena(
            local_buffer.data(), local_buffer.size(), &upstream);
        {
            std::pmr::vector<int> frame_values(&arena);
            for (int value = 0; value < 1024; ++value) {
                frame_values.push_back(value);
            }
            assert(std::accumulate(frame_values.begin(), frame_values.end(), 0) == 523776);
            assert(upstream.allocations() > 0);
            assert(upstream.deallocations() == 0);
        }

        // vector 已销毁，但 monotonic resource 尚未释放上游内存块。
        assert(upstream.deallocations() == 0);
    }

    // arena 销毁后才向上游归还内存块。
    assert(upstream.deallocations() > 0);
    std::cout << "pmr checks passed; upstream allocations=" << upstream.allocations() << '\n';
}
