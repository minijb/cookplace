#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <list>
#include <random>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

using EntityId = std::uint64_t;

struct Entity {
    EntityId id;
    std::uint32_t health;
    float x;
    float y;
    float z;
};

template <typename Function>
auto timed(Function&& function) {
    const auto start = std::chrono::steady_clock::now();
    const auto result = function();
    const auto stop = std::chrono::steady_clock::now();
    const auto microseconds =
        std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
    return std::make_pair(microseconds, result);
}

void print_result(const char* name, const std::pair<long long, std::uint64_t>& result) {
    std::cout << name << ": " << result.first << " us, checksum=" << result.second << '\n';
}

} // namespace

int main() {
    constexpr std::size_t entity_count = 20000;
    constexpr std::size_t query_count = 50000;

    std::vector<Entity> entities;
    entities.reserve(entity_count);
    for (std::size_t index = 0; index < entity_count; ++index) {
        entities.push_back(Entity{EntityId{index * 2 + 1},
            static_cast<std::uint32_t>(index % 100), 1.0F, 2.0F, 3.0F});
    }

    const std::list<Entity> linked_entities(entities.begin(), entities.end());
    std::unordered_map<EntityId, std::size_t> positions;
    positions.reserve(entity_count);
    for (std::size_t index = 0; index < entities.size(); ++index) {
        positions.emplace(entities[index].id, index);
    }

    std::vector<EntityId> queries;
    queries.reserve(query_count);
    std::mt19937_64 random(12345);
    for (std::size_t index = 0; index < query_count; ++index) {
        queries.push_back(EntityId{random() % (entity_count * 3)});
    }

    print_result("vector traversal", timed([&entities] {
        std::uint64_t checksum = 0;
        for (const auto& entity : entities) {
            checksum += entity.health;
        }
        return checksum;
    }));

    print_result("list traversal", timed([&linked_entities] {
        std::uint64_t checksum = 0;
        for (const auto& entity : linked_entities) {
            checksum += entity.health;
        }
        return checksum;
    }));

    print_result("sorted vector binary lookup", timed([&entities, &queries] {
        std::uint64_t checksum = 0;
        for (const auto id : queries) {
            const auto found = std::lower_bound(entities.begin(), entities.end(), id,
                [](const Entity& entity, EntityId key) { return entity.id < key; });
            if (found != entities.end() && found->id == id) {
                checksum += found->health;
            }
        }
        return checksum;
    }));

    print_result("unordered_map lookup", timed([&entities, &positions, &queries] {
        std::uint64_t checksum = 0;
        for (const auto id : queries) {
            const auto found = positions.find(id);
            if (found != positions.end()) {
                checksum += entities[found->second].health;
            }
        }
        return checksum;
    }));
}
