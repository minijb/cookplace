#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>

class Probe {
public:
    Probe(int id, std::vector<int>& events, bool fail = false)
        : id_(id), events_(events) {
        if (fail) {
            throw std::runtime_error("resource acquisition failed");
        }
        ++live_count_;
        events_.push_back(id_);
    }

    ~Probe() {
        --live_count_;
        events_.push_back(-id_);
    }

    Probe(const Probe&) = delete;
    Probe& operator=(const Probe&) = delete;

    static int live_count() {
        return live_count_;
    }

private:
    int id_;
    std::vector<int>& events_;
    inline static int live_count_ = 0;
};

struct ResourcePair {
    std::vector<int>& events;
    Probe first;
    Probe second;

    ResourcePair(std::vector<int>& events, bool fail_second)
        : events(events), first(1, events), second(2, events, fail_second) {}

    ~ResourcePair() {
        events.push_back(99);
    }
};

int main() {
    std::vector<int> events;
    {
        ResourcePair pair(events, false);
        assert(Probe::live_count() == 2);
    }
    assert((events == std::vector<int>{1, 2, 99, -2, -1}));

    events.clear();
    try {
        ResourcePair pair(events, true);
        static_cast<void>(pair);
        assert(false);
    } catch (const std::runtime_error&) {
    }

    assert(Probe::live_count() == 0);
    assert((events == std::vector<int>{1, -1}));
    std::cout << "failed construction cleaned the fully constructed member\n";
}
