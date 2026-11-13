#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

using EntityId = std::uint64_t;

struct Entity {
    EntityId id;
    int health;
    float x;
    float y;
};

class EntityStore {
public:
    bool add(Entity entity) {
        const auto id = entity.id;
        if (positions_.find(id) != positions_.end()) {
            return false;
        }

        entities_.push_back(std::move(entity));
        const auto index = entities_.size() - 1;
        try {
            const auto [position, inserted] = positions_.emplace(id, index);
            assert(inserted);
        } catch (...) {
            entities_.pop_back();
            throw;
        }
        return true;
    }

    Entity* find(EntityId id) {
        const auto position = positions_.find(id);
        return position == positions_.end() ? nullptr : &entities_[position->second];
    }

    const Entity* find(EntityId id) const {
        const auto position = positions_.find(id);
        return position == positions_.end() ? nullptr : &entities_[position->second];
    }

    bool erase(EntityId id) {
        const auto position = positions_.find(id);
        if (position == positions_.end()) {
            return false;
        }

        const auto index = position->second;
        const auto last_index = entities_.size() - 1;
        if (index != last_index) {
            const auto moved_id = entities_.back().id;
            entities_[index] = entities_.back();
            positions_.find(moved_id)->second = index;
        }

        entities_.pop_back();
        positions_.erase(position);
        return true;
    }

    std::size_t erase_dead() {
        const auto old_size = entities_.size();
        std::size_t index = 0;
        while (index < entities_.size()) {
            if (entities_[index].health <= 0) {
                erase(entities_[index].id);
            } else {
                ++index;
            }
        }
        return old_size - entities_.size();
    }

    std::size_t size() const {
        return entities_.size();
    }

private:
    std::vector<Entity> entities_;
    std::unordered_map<EntityId, std::size_t> positions_;
};

void demo_entity_store() {
    EntityStore store;
    assert(store.add(Entity{1, 100, 0.0F, 0.0F}));
    assert(store.add(Entity{2, 0, 1.0F, 0.0F}));
    assert(store.add(Entity{3, 50, 2.0F, 0.0F}));
    assert(!store.add(Entity{1, 10, 0.0F, 0.0F}));

    store.find(3)->health = 45;
    assert(store.erase_dead() == 1);
    assert(store.size() == 2);
    assert(store.find(2) == nullptr);
    assert(store.find(3)->health == 45);

    // 删除 ID 1 后，末尾的 ID 3 被移动到空位，其索引也必须更新。
    assert(store.erase(1));
    assert(store.size() == 1);
    assert(store.find(1) == nullptr);
    assert(store.find(3)->id == 3);
}

void demo_sorted_vector_lookup() {
    std::vector<Entity> sorted{{3, 30, 0.0F, 0.0F}, {1, 10, 0.0F, 0.0F},
        {2, 20, 0.0F, 0.0F}};
    std::sort(sorted.begin(), sorted.end(),
        [](const Entity& lhs, const Entity& rhs) { return lhs.id < rhs.id; });

    const auto found = std::lower_bound(sorted.begin(), sorted.end(), EntityId{2},
        [](const Entity& entity, EntityId id) { return entity.id < id; });
    assert(found != sorted.end() && found->id == 2 && found->health == 20);
}

} // namespace

int main() {
    demo_entity_store();
    demo_sorted_vector_lookup();
    std::cout << "entity storage checks passed\n";
}
