#include "game/Pokemon.hpp"

namespace {
int computestat(int base, int level, bool ishealth) {
    int scaled = ((2 * base + 31) * level) / 100;
    return ishealth ? scaled + level + 10 : scaled + 5;
}
void assignstats(Pokemon& pokemon, int health, int attack, int defense, int specialattack, int specialdefense, int speed) {
    pokemon.maxhealth = computestat(health, pokemon.level, true);  pokemon.health = pokemon.maxhealth;
    pokemon.attack = computestat(attack, pokemon.level, false);  pokemon.defense = computestat(defense, pokemon.level, false);
    pokemon.specialattack = computestat(specialattack, pokemon.level, false);  pokemon.specialdefense = computestat(specialdefense, pokemon.level, false);
    pokemon.speed = computestat(speed, pokemon.level, false);
}
}

Pokemon::Pokemon() : name(), type1(Type::Normal), type2(Type::None), level(50),
    maxhealth(1), health(1), attack(1), defense(1), specialattack(1), specialdefense(1), speed(1), movecount(0), status() {}

Pokemon Pokemon::makecharizard() {
    Pokemon charizard;
    charizard.name = "Charizard"; charizard.type1 = Type::Fire; charizard.type2 = Type::Flying; charizard.level = 50;
    assignstats(charizard, 78, 84, 78, 109, 85, 100);
    charizard.moves[0] = "Flamethrower"; charizard.moves[1] = "Dragon Claw";
    charizard.moves[2] = "Air Slash";    charizard.moves[3] = "Earthquake";
    charizard.movecount = 4;
    return charizard;
}

Pokemon Pokemon::makesceptile() {
    Pokemon sceptile;
    sceptile.name = "Sceptile"; sceptile.type1 = Type::Grass; sceptile.type2 = Type::None; sceptile.level = 50;
    assignstats(sceptile, 70, 85, 65, 105, 85, 120);
    sceptile.moves[0] = "Leaf Blade";    sceptile.moves[1] = "Dragon Pulse";
    sceptile.moves[2] = "Quick Attack";  sceptile.moves[3] = "Thunder Punch";
    sceptile.movecount = 4;
    return sceptile;
}

int Pokemon::effectivespeed() const {
    return status.has(StatusType::Paralysis) ? speed / 2 : speed;
}

bool Pokemon::caninflict(StatusType condition) const {
    if (status.hasany() || condition == StatusType::None) return false;
    if (condition == StatusType::Burn && (type1 == Type::Fire || type2 == Type::Fire)) return false;
    if (condition == StatusType::Paralysis && (type1 == Type::Electric || type2 == Type::Electric)) return false;
    return true;
}

PokemonState Pokemon::savestate() const {
    PokemonState saved;
    saved.health = health;
    saved.statuscount = status.snapshot(saved.statuses, StatusQueue::maxsnapshot);
    return saved;
}

void Pokemon::loadstate(const PokemonState& saved) {
    health = saved.health;
    status.restore(saved.statuses, saved.statuscount);
}
