#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

class GpuHandle {
public:
    explicit GpuHandle(int id) : id_(id) {
        ++live_count_;
    }

    ~GpuHandle() {
        release();
    }

    GpuHandle(const GpuHandle&) = delete;
    GpuHandle& operator=(const GpuHandle&) = delete;

    GpuHandle(GpuHandle&& other) noexcept
        : id_(std::exchange(other.id_, -1)) {}

    GpuHandle& operator=(GpuHandle&& other) noexcept {
        if (this != &other) {
            release();
            id_ = std::exchange(other.id_, -1);
        }
        return *this;
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

struct Texture {
    std::string name;
    GpuHandle handle;
};

class ResourceManager {
public:
    Texture& acquire(std::string_view name) {
        if (name.empty()) {
            throw std::invalid_argument("resource name cannot be empty");
        }

        std::string key(name);
        auto it = textures_.find(key);
        if (it == textures_.end()) {
            auto texture = std::make_unique<Texture>(
                Texture{key, GpuHandle(next_id_++)});
            it = textures_.emplace(key, std::move(texture)).first;
        }
        return *it->second;
    }

    std::size_t size() const {
        return textures_.size();
    }

    void clear() {
        textures_.clear();
    }

private:
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures_;
    int next_id_ = 1;
};

int main() {
    {
        ResourceManager manager;
        Texture& player = manager.acquire("player");
        Texture& same_player = manager.acquire("player");
        assert(&player == &same_player);
        assert(manager.size() == 1);
        assert(GpuHandle::live_count() == 1);

        manager.acquire("enemy");
        assert(GpuHandle::live_count() == 2);
        std::cout << "player handle=" << player.handle.id() << '\n';

        manager.clear();
        assert(GpuHandle::live_count() == 0);

        try {
            manager.acquire("");
            assert(false);
        } catch (const std::invalid_argument&) {
        }
        assert(GpuHandle::live_count() == 0);

        manager.acquire("terrain");
        assert(GpuHandle::live_count() == 1);
    }
    assert(GpuHandle::live_count() == 0);
    std::cout << "manager destruction released all GPU handles\n";
}
