#include "game/StatusQueue.hpp"
using namespace std;

namespace {
struct StatusFinder {
    StatusType wanted; bool found;
    void operator()(const StatusEntry& entry) { if (entry.type == wanted) found = true; }
};
struct StatusCopier {
    StatusEntry* output; int limit; int copied;
    void operator()(const StatusEntry& entry) { if (copied < limit) output[copied++] = entry; }
};
}

bool StatusQueue::has(StatusType type) const {
    StatusFinder finder = { type, false };
    entries.visitall(finder);
    return finder.found;
}

StatusType StatusQueue::current() const {
    const StatusEntry* entry = entries.front();
    return entry ? entry->type : StatusType::None;
}

void StatusQueue::tick(int maxhealth, int& damage, StatusType& expired) {
    damage = 0;
    expired = StatusType::None;
    size_t total = entries.size();
    for (size_t i = 0; i < total; ++i) {
        StatusEntry entry;
        entries.dequeue(entry);
        if (entry.type == StatusType::Burn) {
            int burn = maxhealth / 16;
            damage += (burn < 1 ? 1 : burn);
        }
        if (--entry.turnsleft > 0) entries.enqueue(entry);
        else expired = entry.type;
    }
}

int StatusQueue::snapshot(StatusEntry* output, int maxcount) const {
    StatusCopier copier = { output, maxcount, 0 };
    entries.visitall(copier);
    return copier.copied;
}

void StatusQueue::restore(const StatusEntry* input, int count) {
    entries.clear();
    for (int i = 0; i < count; ++i) entries.enqueue(input[i]);
}
