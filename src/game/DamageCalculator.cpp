#include "game/DamageCalculator.hpp"

namespace { const float damagescale = 0.6f; }

float DamageCalculator::totaleffectiveness(const Move& chosenmove, const Pokemon& defender) {
    float total = typematchup(chosenmove.type, defender.type1);
    if (defender.type2 != Type::None) total *= typematchup(chosenmove.type, defender.type2);
    return total;
}

DamageResult DamageCalculator::compute(const Pokemon& attacker, const Pokemon& defender, const Move& chosenmove,
                                       bool critical, float randomfactor) {
    DamageResult result;
    result.damage = 0;
    result.effectiveness = totaleffectiveness(chosenmove, defender);
    result.critical = critical;
    result.typebonus = (chosenmove.type == attacker.type1 || chosenmove.type == attacker.type2);
    if (chosenmove.power <= 0 || result.effectiveness == 0.0f) return result;

    const bool physical = (chosenmove.category == MoveCategory::Physical);
    const float attackstat = (float)(physical ? attacker.attack : attacker.specialattack);
    const float defensestat = (float)(physical ? defender.defense : defender.specialdefense);
    const float basedamage = ((2.0f * attacker.level / 5.0f + 2.0f) * chosenmove.power * attackstat / defensestat) / 50.0f + 2.0f;

    float modifier = randomfactor * result.effectiveness * damagescale;
    if (result.typebonus) modifier *= 1.5f;
    if (critical) modifier *= 1.5f;
    if (physical && attacker.status.has(StatusType::Burn)) modifier *= 0.5f;

    int finaldamage = (int)(basedamage * modifier);
    result.damage = finaldamage < 1 ? 1 : finaldamage;
    return result;
}

int DamageCalculator::estimate(const Pokemon& attacker, const Pokemon& defender, const Move& chosenmove) {
    return compute(attacker, defender, chosenmove, false, 0.925f).damage;
}
