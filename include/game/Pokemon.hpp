#pragma once
#include <string>
#include "game/Types.hpp"
#include "game/StatusQueue.hpp"
using namespace std;

struct PokemonState {
    int health;
    StatusEntry statuses[StatusQueue::maxsnapshot];
    int statuscount;
    PokemonState() : health(0), statuscount(0) {}
};

class Pokemon {
public:
    string name;
    Type type1, type2;
    int level;
    int maxhealth, health, attack, defense, specialattack, specialdefense, speed;
    string moves[4];
    int movecount;
    StatusQueue status;

    Pokemon();
    static Pokemon makecharizard();
    static Pokemon makesceptile();

    bool fainted() const { return health <= 0; }
    float healthfraction() const { return maxhealth > 0 ? (float)health / (float)maxhealth : 0.f; }
    int effectivespeed() const;
    void takedamage(int amount) { health = (amount >= health) ? 0 : health - amount; }
    // No double status; Fire is immune to burn, Electric to paralysis.
    bool caninflict(StatusType condition) const;
    PokemonState savestate() const;
    void loadstate(const PokemonState& saved);
};
