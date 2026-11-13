#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

class Buffer {
public:
    explicit Buffer(std::size_t size)
        : data_(size == 0 ? nullptr : new std::uint8_t[size]), size_(size) {
        if (size_ != 0) {
            std::fill_n(data_, size_, std::uint8_t{0});
        }
    }

    ~Buffer() {
        delete[] data_;
    }

    Buffer(const Buffer& other)
        : data_(other.size_ == 0 ? nullptr : new std::uint8_t[other.size_]),
          size_(other.size_) {
        if (size_ != 0) {
            std::copy_n(other.data_, size_, data_);
        }
    }

    Buffer& operator=(const Buffer& other) {
        Buffer copy(other);
        swap(copy);
        return *this;
    }

    Buffer(Buffer&& other) noexcept
        : data_(std::exchange(other.data_, nullptr)),
          size_(std::exchange(other.size_, 0)) {}

    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = std::exchange(other.data_, nullptr);
            size_ = std::exchange(other.size_, 0);
        }
        return *this;
    }

    void swap(Buffer& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
    }

    std::size_t size() const {
        return size_;
    }

    std::uint8_t* data() {
        return data_;
    }

private:
    std::uint8_t* data_;
    std::size_t size_;
};

template <typename Operation>
double measure_ms(Operation operation) {
    const auto start = std::chrono::steady_clock::now();
    operation();
    const auto finish = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(finish - start).count();
}

int main() {
    constexpr std::size_t byte_count = 1024 * 1024;
    constexpr std::size_t buffer_count = 16;

    Buffer source(byte_count);
    source.data()[0] = 42;

    std::vector<Buffer> copies;
    copies.reserve(buffer_count);
    const double copy_ms = measure_ms([&] {
        for (std::size_t i = 0; i < buffer_count; ++i) {
            copies.push_back(source);
        }
    });

    std::vector<Buffer> sources;
    sources.reserve(buffer_count);
    for (std::size_t i = 0; i < buffer_count; ++i) {
        Buffer item(byte_count);
        item.data()[0] = 42;
        sources.push_back(std::move(item));
    }

    std::vector<Buffer> moves;
    moves.reserve(buffer_count);
    const double move_ms = measure_ms([&] {
        for (auto& item : sources) {
            moves.push_back(std::move(item));
        }
    });

    assert(copies.size() == buffer_count);
    assert(moves.size() == buffer_count);
    assert(copies.back().data()[0] == 42);
    assert(moves.back().data()[0] == 42);

    std::cout << "copy " << buffer_count << " x " << byte_count
              << " bytes: " << copy_ms << " ms\n";
    std::cout << "move " << buffer_count << " x " << byte_count
              << " bytes: " << move_ms << " ms\n";
    std::cout << "measurements are environment-dependent; repeat before drawing conclusions\n";
}
