#pragma once
#include "game/Pokemon.hpp"
#include "game/Move.hpp"

struct DamageResult {
    int damage;
    float effectiveness;
    bool critical;
    bool typebonus;
};

class DamageCalculator {
public:
    // ((2L/5+2) * Power * A/D)/50 + 2, then x random x same-type bonus x type matchup x critical x burn.
    static DamageResult compute(const Pokemon& attacker, const Pokemon& defender, const Move& chosenmove,
                                bool critical, float randomfactor);
    static int estimate(const Pokemon& attacker, const Pokemon& defender, const Move& chosenmove);
    static float totaleffectiveness(const Move& chosenmove, const Pokemon& defender);
};
