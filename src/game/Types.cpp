#include "game/Types.hpp"

namespace {
struct Table {
    float values[typecount][typecount];
    void assign(Type attacker, Type defender, float multiplier) { values[(int)attacker][(int)defender] = multiplier; }
    Table() {
        for (int row = 0; row < typecount; ++row)
            for (int column = 0; column < typecount; ++column) values[row][column] = 1.0f;
        assign(Type::Fire, Type::Grass, 2); assign(Type::Fire, Type::Fire, .5f);
        assign(Type::Fire, Type::Water, .5f); assign(Type::Fire, Type::Dragon, .5f);
        assign(Type::Water, Type::Fire, 2); assign(Type::Water, Type::Ground, 2);
        assign(Type::Water, Type::Water, .5f); assign(Type::Water, Type::Grass, .5f);
        assign(Type::Water, Type::Dragon, .5f);
        assign(Type::Grass, Type::Water, 2); assign(Type::Grass, Type::Ground, 2);
        assign(Type::Grass, Type::Fire, .5f); assign(Type::Grass, Type::Grass, .5f);
        assign(Type::Grass, Type::Flying, .5f); assign(Type::Grass, Type::Dragon, .5f);
        assign(Type::Electric, Type::Water, 2); assign(Type::Electric, Type::Flying, 2);
        assign(Type::Electric, Type::Electric, .5f); assign(Type::Electric, Type::Grass, .5f);
        assign(Type::Electric, Type::Dragon, .5f); assign(Type::Electric, Type::Ground, 0);
        assign(Type::Flying, Type::Grass, 2); assign(Type::Flying, Type::Electric, .5f);
        assign(Type::Dragon, Type::Dragon, 2);
        assign(Type::Ground, Type::Fire, 2); assign(Type::Ground, Type::Electric, 2);
        assign(Type::Ground, Type::Grass, .5f); assign(Type::Ground, Type::Flying, 0);
    }
};
}

float typematchup(Type attacker, Type defender) {
    static const Table table;
    if (attacker == Type::None || defender == Type::None) return 1.0f;
    return table.values[(int)attacker][(int)defender];
}

const char* nameoftype(Type type) {
    switch (type) {
        case Type::Normal: return "Normal";     case Type::Fire: return "Fire";
        case Type::Water: return "Water";       case Type::Grass: return "Grass";
        case Type::Electric: return "Electric"; case Type::Flying: return "Flying";
        case Type::Dragon: return "Dragon";     case Type::Ground: return "Ground";
        default: return "-";
    }
}

const char* nameofstatus(StatusType status) {
    switch (status) {
        case StatusType::Burn: return "Burn";
        case StatusType::Paralysis: return "Paralysis";
        default: return "None";
    }
}
