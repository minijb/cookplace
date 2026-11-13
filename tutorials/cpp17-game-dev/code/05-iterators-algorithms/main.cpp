#include <algorithm>
#include <cassert>
#include <forward_list>
#include <iostream>
#include <iterator>
#include <list>
#include <numeric>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace {

struct Entity {
    int id;
    int health;
    std::string name;
};

static_assert(std::is_same_v<
    std::iterator_traits<std::vector<int>::iterator>::iterator_category,
    std::random_access_iterator_tag>);
static_assert(std::is_same_v<
    std::iterator_traits<std::list<int>::iterator>::iterator_category,
    std::bidirectional_iterator_tag>);
static_assert(std::is_same_v<
    std::iterator_traits<std::forward_list<int>::iterator>::iterator_category,
    std::forward_iterator_tag>);
static_assert(std::is_same_v<
    std::iterator_traits<std::istream_iterator<int>>::iterator_category,
    std::input_iterator_tag>);

bool contains(std::string_view text, std::string_view fragment) {
    return text.find(fragment) != std::string_view::npos;
}

void demo_algorithms() {
    std::vector<Entity> entities{
        {3, 0, "wisp"}, {1, 50, "player"}, {2, 20, "orc"}, {4, 50, "boss"}};

    const auto dead = std::find_if(entities.begin(), entities.end(),
        [](const Entity& entity) { return entity.health <= 0; });
    assert(dead != entities.end() && dead->id == 3);

    const int damage_threshold = 30;
    const auto wounded = std::count_if(entities.begin(), entities.end(),
        [damage_threshold](const Entity& entity) {
            return entity.health > 0 && entity.health < damage_threshold;
        });
    assert(wounded == 1);

    std::sort(entities.begin(), entities.end(),
        [](const Entity& lhs, const Entity& rhs) {
            if (lhs.health != rhs.health) {
                return lhs.health > rhs.health;
            }
            return lhs.id < rhs.id;
        });
    assert(entities.front().id == 1);
    assert(entities[1].id == 4);

    std::vector<std::string> names(entities.size());
    std::transform(entities.begin(), entities.end(), names.begin(),
        [](const Entity& entity) -> const std::string& { return entity.name; });
    assert(names.front() == "player");

    const auto total_health = std::accumulate(entities.begin(), entities.end(), 0,
        [](int total, const Entity& entity) { return total + entity.health; });
    assert(total_health == 120);

    const auto old_size = entities.size();
    const auto new_end = std::remove_if(entities.begin(), entities.end(),
        [](const Entity& entity) { return entity.health <= 0; });
    assert(entities.size() == old_size);
    entities.erase(new_end, entities.end());
    assert(entities.size() == old_size - 1);

    std::sort(entities.begin(), entities.end(),
        [](const Entity& lhs, const Entity& rhs) { return lhs.id < rhs.id; });
    const auto entity_two = std::lower_bound(entities.begin(), entities.end(), 2,
        [](const Entity& entity, int id) { return entity.id < id; });
    assert(entity_two != entities.end() && entity_two->id == 2);
}

void demo_output_iterators_and_list_members() {
    const std::vector<int> source{1, 2, 3};
    std::vector<int> doubled;
    doubled.reserve(source.size());
    std::transform(source.begin(), source.end(), std::back_inserter(doubled),
        [](int value) { return value * 2; });
    assert((doubled == std::vector<int>{2, 4, 6}));

    std::list<int> values{1, 2, 3, 4};
    values.remove_if([](int value) { return value % 2 == 0; });
    assert((values == std::list<int>{1, 3}));
}

void demo_string_view() {
    std::string owner = "player:fire";
    const std::string_view view = owner;
    assert(contains(view, "fire"));
    assert(view.substr(0, 6) == "player");

    const std::string copied{view};
    assert(copied == owner);
}

} // namespace

int main() {
    demo_algorithms();
    demo_output_iterators_and_list_members();
    demo_string_view();
    std::cout << "iterator and algorithm checks passed\n";
}
