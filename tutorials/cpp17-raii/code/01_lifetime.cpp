#include <cassert>
#include <iostream>
#include <vector>

class Ticket {
public:
    Ticket(int id, std::vector<int>& events) : id_(id), events_(events) {
        ++live_count_;
        events_.push_back(id_);
    }

    ~Ticket() {
        --live_count_;
        events_.push_back(-id_);
    }

    Ticket(const Ticket&) = delete;
    Ticket& operator=(const Ticket&) = delete;

    static int live_count() {
        return live_count_;
    }

private:
    int id_;
    std::vector<int>& events_;
    inline static int live_count_ = 0;
};

struct FrameResources {
    Ticket texture;
    Ticket vertex_buffer;
};

int main() {
    std::vector<int> events;
    {
        FrameResources resources{{1, events}, {2, events}};
        assert(Ticket::live_count() == 2);
        assert((events == std::vector<int>{1, 2}));
    }

    assert(Ticket::live_count() == 0);
    assert((events == std::vector<int>{1, 2, -2, -1}));

    std::cout << "construction/destruction events:";
    for (const int event : events) {
        std::cout << ' ' << event;
    }
    std::cout << '\n';
}
