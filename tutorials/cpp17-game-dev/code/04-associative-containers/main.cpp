#include <cassert>
#include <cstddef>
#include <deque>
#include <functional>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

struct Player {
    int score;
    int id;
};

struct ByScore {
    bool operator()(const Player& lhs, const Player& rhs) const {
        if (lhs.score != rhs.score) {
            return lhs.score > rhs.score;
        }
        return lhs.id < rhs.id;
    }
};

void demo_ordered_map_and_node_handle() {
    std::map<int, std::string> names{{3, "wisp"}, {1, "player"}, {2, "orc"}};

    std::vector<int> visit_order;
    for (const auto& [id, name] : names) {
        visit_order.push_back(id);
        assert(!name.empty());
    }
    assert((visit_order == std::vector<int>{1, 2, 3}));

    const auto [position, inserted] = names.try_emplace(2, "replacement");
    assert(!inserted && position->second == "orc");

    names.insert_or_assign(2, "elite orc");
    assert(names.at(2) == "elite orc");

    auto node = names.extract(1);
    assert(node && names.find(1) == names.end());
    node.key() = 10;

    std::map<int, std::string> archive;
    archive.insert(std::move(node));
    assert(node.empty());
    assert(archive.at(10) == "player");
}

void demo_strict_weak_ordering() {
    std::set<Player, ByScore> ranking{{100, 2}, {80, 3}, {100, 1}};
    const auto first = ranking.begin();
    assert(first->score == 100 && first->id == 1);
}

void demo_unordered_map() {
    std::unordered_map<int, std::string> entities;
    entities.max_load_factor(0.75F);
    entities.reserve(4);
    for (int id = 0; id < 4; ++id) {
        entities.try_emplace(id, "entity-" + std::to_string(id));
    }

    const std::string* entity_two = &entities.at(2);
    const auto old_bucket_count = entities.bucket_count();
    entities.rehash(old_bucket_count * 4 + 1);

    assert(entities.bucket_count() > old_bucket_count);
    // rehash 使迭代器失效，但不使元素指针或引用失效。
    assert(entity_two == &entities.at(2));
    assert(*entity_two == "entity-2");
}

void demo_multimap_and_adapters() {
    std::multimap<int, std::string> spawns{{1, "north"}, {2, "south"}, {1, "cave"}};
    const auto [first, last] = spawns.equal_range(1);
    assert(std::distance(first, last) == 2);

    std::stack<int, std::vector<int>> undo_stack;
    undo_stack.push(7);
    assert(undo_stack.top() == 7);

    std::queue<int, std::deque<int>> commands;
    commands.push(11);
    assert(commands.front() == 11);

    std::priority_queue<int> max_heap;
    max_heap.push(3);
    max_heap.push(9);
    assert(max_heap.top() == 9);

    std::priority_queue<int, std::vector<int>, std::greater<int>> min_heap;
    min_heap.push(3);
    min_heap.push(9);
    assert(min_heap.top() == 3);
}

} // namespace

int main() {
    demo_ordered_map_and_node_handle();
    demo_strict_weak_ordering();
    demo_unordered_map();
    demo_multimap_and_adapters();
    std::cout << "associative container checks passed\n";
}
