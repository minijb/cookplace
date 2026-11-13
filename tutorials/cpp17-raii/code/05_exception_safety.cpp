#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

struct PlayerEvent {
    std::string player_name;
    std::string item;
};

class PlayerState {
public:
    void apply(const PlayerEvent& event) {
        PlayerState updated(*this);
        updated.player_name_ = event.player_name;
        if (event.item == "invalid") {
            throw std::invalid_argument("invalid item");
        }
        updated.inventory_.push_back(event.item);
        swap(updated);
    }

    void swap(PlayerState& other) noexcept {
        std::swap(player_name_, other.player_name_);
        std::swap(inventory_, other.inventory_);
    }

    const std::string& player_name() const {
        return player_name_;
    }

    const std::vector<std::string>& inventory() const {
        return inventory_;
    }

private:
    std::string player_name_ = "anonymous";
    std::vector<std::string> inventory_;
};

int main() {
    PlayerState state;
    try {
        state.apply({"alice", "invalid"});
        assert(false);
    } catch (const std::invalid_argument&) {
    }

    assert(state.player_name() == "anonymous");
    assert(state.inventory().empty());

    state.apply({"alice", "potion"});
    assert(state.player_name() == "alice");
    assert(state.inventory().size() == 1);
    assert(state.inventory()[0] == "potion");

    std::cout << "failed update left the original state unchanged\n";
}
