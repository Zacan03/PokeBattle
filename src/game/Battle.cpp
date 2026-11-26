#include "game/Battle.hpp"
#include "game/DamageCalculator.hpp"
using namespace std;

Battle::Battle(uint32_t seed)
    : database(), playeronepokemon(Pokemon::makecharizard()), playertwopokemon(Pokemon::makesceptile()),
      history(), eventqueue(), masterrng(seed), turn(1), stage(Stage::NotStarted),
      winnerisplayerone(false), pendingmove(-1) {}

void Battle::reset(uint32_t seed) {
    playeronepokemon = Pokemon::makecharizard();
    playertwopokemon = Pokemon::makesceptile();
    history.clear();
    eventqueue.clear();
    masterrng = Rng(seed);
    turn = 1; stage = Stage::NotStarted; winnerisplayerone = false; pendingmove = -1;
}

void Battle::emit(EventType kind, bool actorisplayerone, bool targetisplayerone, const string& text, int amount,
                  float multiplier, bool critical, Type movetype, const string& movename) {
    BattleEvent event;
    event.type = kind; event.actorisplayerone = actorisplayerone; event.targetisplayerone = targetisplayerone; event.text = text;
    event.amount = amount; event.effectiveness = multiplier; event.critical = critical; event.movetype = movetype; event.movename = movename;
    event.playeronehealth = playeronepokemon.health; event.playertwohealth = playertwopokemon.health;
    event.playeronestatus = playeronepokemon.status.current(); event.playertwostatus = playertwopokemon.status.current();
    eventqueue.enqueue(event);
}

void Battle::begin() {
    if (stage != Stage::NotStarted) return;
    stage = Stage::P1Select;
    emit(EventType::TurnStart, true, true, "", 0);
}

PromptState Battle::captureprompt() const {
    PromptState state;
    state.playerone = playeronepokemon.savestate();
    state.playertwo = playertwopokemon.savestate();
    state.turn = turn;
    return state;
}

void Battle::restorefrom(const PromptState& state) {
    playeronepokemon.loadstate(state.playerone);
    playertwopokemon.loadstate(state.playertwo);
    turn = state.turn;
    winnerisplayerone = false;
    pendingmove = -1;
    stage = Stage::P1Select;
}

bool Battle::submitmove(int index) {
    if (stage == Stage::P1Select) {
        if (index < 0 || index >= playeronepokemon.movecount) return false;
        pendingmove = index;
        stage = Stage::P2Select;
        emit(EventType::Log, true, true, playeronepokemon.name + " has chosen a move.");
        emit(EventType::TurnStart, false, false, "", 0);
        return true;
    }
    if (stage == Stage::P2Select) {
        if (index < 0 || index >= playertwopokemon.movecount) return false;
        TurnRecord record;
        record.before = captureprompt();
        record.playeronemove = pendingmove;
        record.playertwomove = index;
        record.seed = masterrng.next();
        history.push(record);
        runround(record.playeronemove, record.playertwomove, record.seed);
        return true;
    }
    return false;
}

bool Battle::undolastturn() {
    if (stage == Stage::P2Select) {
        pendingmove = -1;
        stage = Stage::P1Select;
        emit(EventType::Sync, true, true, playeronepokemon.name + "'s choice cancelled.");
        emit(EventType::TurnStart, true, true, "", 0);
        return true;
    }
    TurnRecord record;
    if (!history.pop(record)) return false;
    restorefrom(record.before);
    emit(EventType::Sync, true, true, "Turn " + to_string(record.before.turn) + " undone - rewinding!");
    emit(EventType::TurnStart, true, true, "", 0);
    return true;
}

bool Battle::replaylastturn() {
    const TurnRecord* latest = history.top();
    if (!latest) return false;
    TurnRecord record = *latest;
    restorefrom(record.before);
    emit(EventType::Sync, true, true, "Replaying turn " + to_string(record.before.turn) + "...");
    runround(record.playeronemove, record.playertwomove, record.seed);
    return true;
}

void Battle::runround(int playeronemove, int playertwomove, uint32_t seed) {
    Rng rng(seed);
    stage = Stage::Resolving;
    const Move* playeronedata = database.get(playeronepokemon.moves[playeronemove]);
    const Move* playertwodata = database.get(playertwopokemon.moves[playertwomove]);
    if (!playeronedata || !playertwodata) { stage = Stage::P1Select; return; }

    PriorityQueue<TurnAction, TurnActionLess> order;
    TurnAction playeroneaction;
    playeroneaction.bracket = playeronedata->priority; playeroneaction.speed = playeronepokemon.effectivespeed();
    playeroneaction.tiebreak = (int)(rng.next() % 1000); playeroneaction.isplayerone = true; playeroneaction.moveindex = playeronemove;
    order.push(playeroneaction);
    TurnAction playertwoaction;
    playertwoaction.bracket = playertwodata->priority; playertwoaction.speed = playertwopokemon.effectivespeed();
    playertwoaction.tiebreak = (int)(rng.next() % 1000); playertwoaction.isplayerone = false; playertwoaction.moveindex = playertwomove;
    order.push(playertwoaction);

    TurnAction action;
    while (stage != Stage::Over && order.pop(action)) {
        emit(EventType::TurnStart, action.isplayerone, action.isplayerone, "", 1);
        performaction(action, rng);
    }
    if (stage != Stage::Over) endofturn();
    if (stage != Stage::Over) {
        ++turn;
        pendingmove = -1;
        stage = Stage::P1Select;
        emit(EventType::TurnStart, true, true, "", 0);
    }
}

bool Battle::finishiffainted(bool playeroneside) {
    Pokemon& pokemon = playeroneside ? playeronepokemon : playertwopokemon;
    if (!pokemon.fainted()) return false;
    emit(EventType::Faint, playeroneside, playeroneside, pokemon.name + " fainted!");
    stage = Stage::Over;
    winnerisplayerone = !playeroneside;
    emit(EventType::BattleEnd, !playeroneside, !playeroneside, (winnerisplayerone ? playeronepokemon.name : playertwopokemon.name) + " wins the battle!");
    return true;
}

void Battle::performaction(const TurnAction& action, Rng& rng) {
    Pokemon& attacker = action.isplayerone ? playeronepokemon : playertwopokemon;
    Pokemon& defender = action.isplayerone ? playertwopokemon : playeronepokemon;
    if (attacker.fainted()) return;
    const Move* usedmove = database.get(attacker.moves[action.moveindex]);
    if (!usedmove) return;

    if (attacker.status.has(StatusType::Paralysis) && rng.range(1, 100) <= 25) {
        emit(EventType::Log, action.isplayerone, action.isplayerone, attacker.name + " is fully paralyzed!");
        return;
    }
    emit(EventType::MoveUsed, action.isplayerone, !action.isplayerone, attacker.name + " used " + usedmove->name + "!",
         0, 1.f, false, usedmove->type, usedmove->name);

    if (rng.range(1, 100) > usedmove->accuracy) {
        emit(EventType::Miss, action.isplayerone, !action.isplayerone, attacker.name + "'s attack missed!");
        return;
    }

    bool landed = true;
    if (usedmove->power > 0) {
        bool critical = rng.range(1, 16) == 1;
        float randomfactor = 0.85f + 0.15f * rng.unit();
        DamageResult result = DamageCalculator::compute(attacker, defender, *usedmove, critical, randomfactor);
        if (result.effectiveness == 0.f) {
            landed = false;
            emit(EventType::Log, action.isplayerone, !action.isplayerone, "It doesn't affect " + defender.name + "...");
        } else {
            defender.takedamage(result.damage);
            string message;
            if (critical) message += "A critical hit! ";
            if (result.effectiveness >= 2.f) message += "It's super effective!";
            else if (result.effectiveness < 1.f) message += "It's not very effective...";
            emit(EventType::Damage, action.isplayerone, !action.isplayerone, message, result.damage, result.effectiveness, critical, usedmove->type);
        }
    }

    if (landed && !defender.fainted() && usedmove->status != StatusType::None && defender.caninflict(usedmove->status)
        && rng.range(1, 100) <= usedmove->statuschance) {
        defender.status.add(usedmove->status, usedmove->status == StatusType::Burn ? 5 : 4);
        emit(EventType::StatusApplied, !action.isplayerone, !action.isplayerone,
             usedmove->status == StatusType::Burn ? defender.name + " was burned!"
                                                  : defender.name + " is paralyzed! It may be unable to move!",
             0, 1.f, false, usedmove->type);
    }
    finishiffainted(!action.isplayerone);
}

void Battle::endofturn() {
    for (int side = 0; side < 2 && stage != Stage::Over; ++side) {
        const bool isplayerone = (side == 0);
        Pokemon& pokemon = isplayerone ? playeronepokemon : playertwopokemon;
        if (pokemon.fainted() || !pokemon.status.hasany()) continue;
        int damage = 0;
        StatusType expired = StatusType::None;
        StatusType currentstatus = pokemon.status.current();
        pokemon.status.tick(pokemon.maxhealth, damage, expired);
        if (damage > 0) {
            pokemon.takedamage(damage);
            emit(EventType::StatusDamage, isplayerone, isplayerone,
                 pokemon.name + " is hurt by its " + (currentstatus == StatusType::Burn ? "burn" : "condition") + "!", damage);
            if (finishiffainted(isplayerone)) break;
        }
        if (expired != StatusType::None)
            emit(EventType::StatusExpired, isplayerone, isplayerone,
                 pokemon.name + (expired == StatusType::Burn ? "'s burn faded." : "'s paralysis wore off!"));
    }
}
