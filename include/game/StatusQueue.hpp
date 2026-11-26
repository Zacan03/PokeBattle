#pragma once
#include "core/Queue.hpp"
#include "game/Types.hpp"

struct StatusEntry {
    StatusType type;
    int turnsleft;
    StatusEntry() : type(StatusType::None), turnsleft(0) {}
    StatusEntry(StatusType giventype, int giventurns) : type(giventype), turnsleft(giventurns) {}
};

class StatusQueue {
    Queue<StatusEntry> entries;
public:
    static const int maxsnapshot = 4;

    bool hasany() const { return !entries.empty(); }
    bool has(StatusType type) const;
    StatusType current() const;
    void add(StatusType type, int turns) { entries.enqueue(StatusEntry(type, turns)); }
    void clear() { entries.clear(); }

    // damage = HP lost this tick; expired = type that ended this tick (or None).
    void tick(int maxhealth, int& damage, StatusType& expired);

    int snapshot(StatusEntry* output, int maxcount) const;
    void restore(const StatusEntry* input, int count);
};
