#pragma once
#include "core/HashTable.hpp"
#include "game/Move.hpp"
using namespace std;

class MoveDatabase {
    HashTable<Move> table;
public:
    MoveDatabase();
    const Move* get(const string& name) const { return table.find(name); }
    size_t size() const { return table.size(); }
};
