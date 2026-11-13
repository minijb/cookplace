#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
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

    const std::uint8_t* data() const {
        return data_;
    }

    std::uint8_t& at(std::size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Buffer index out of range");
        }
        return data_[index];
    }

    const std::uint8_t& at(std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Buffer index out of range");
        }
        return data_[index];
    }

private:
    std::uint8_t* data_;
    std::size_t size_;
};

static_assert(std::is_nothrow_move_constructible_v<Buffer>);
static_assert(std::is_nothrow_move_assignable_v<Buffer>);

struct Resource {
    std::string name;
    Buffer payload;

    Resource(std::string resource_name, std::size_t size)
        : name(std::move(resource_name)), payload(size) {}
};

std::unique_ptr<Resource> load_unique(const std::string& name) {
    return std::make_unique<Resource>(name, 64);
}

void print_resource(const Resource& resource) {
    std::cout << "resource " << resource.name << ": bytes="
              << resource.payload.size() << '\n';
}

void self_move(Buffer& value) {
    value = std::move(value);
}

class ResourceCache {
public:
    std::shared_ptr<Resource> acquire(const std::string& name) {
        const auto it = entries_.find(name);
        if (it != entries_.end()) {
            if (auto existing = it->second.lock()) {
                return existing;
            }
            entries_.erase(it);
        }

        auto resource = std::make_shared<Resource>(name, 128);
        entries_.emplace(name, resource);
        return resource;
    }

private:
    std::unordered_map<std::string, std::weak_ptr<Resource>> entries_;
};

int main() {
    Buffer original(4);
    for (std::size_t i = 0; i < original.size(); ++i) {
        original.at(i) = static_cast<std::uint8_t>(i + 1);
    }

    Buffer copy(original);
    copy.at(0) = 99;
    assert(original.at(0) == 1);
    assert(copy.at(0) == 99);

    Buffer moved(std::move(copy));
    assert(copy.size() == 0);
    assert(moved.at(0) == 99);

    Buffer assigned(2);
    assigned = std::move(moved);
    assert(moved.size() == 0);
    assert(assigned.size() == 4);

    self_move(assigned);
    assert(assigned.size() == 4 && assigned.at(0) == 99);

    std::vector<Buffer> buffers;
    buffers.reserve(1);
    buffers.emplace_back(8);
    buffers.emplace_back(16);
    assert(buffers.size() == 2);

    auto unique = load_unique("player");
    print_resource(*unique);
    auto transferred = std::move(unique);
    assert(unique == nullptr);
    assert(transferred != nullptr);

    ResourceCache cache;
    auto player = cache.acquire("player");
    std::weak_ptr<Resource> observer = player;
    {
        auto same_player = cache.acquire("player");
        assert(same_player.get() == player.get());
        assert(player.use_count() == 2);
        print_resource(*same_player);
    }

    assert(player.use_count() == 1);
    assert(!observer.expired());
    player.reset();
    assert(observer.expired());

    std::cout << "copy, move, unique ownership, and weak cache checks passed\n";
}
