#pragma once
#include <cstdint>
#include <string>
#include "core/Stack.hpp"
#include "core/Queue.hpp"
#include "core/PriorityQueue.hpp"
#include "game/Pokemon.hpp"
#include "game/MoveDatabase.hpp"
#include "game/Rng.hpp"
using namespace std;

enum class EventType { Log, MoveUsed, Damage, Miss, StatusApplied, StatusDamage,
                       StatusExpired, Faint, Sync, BattleEnd, TurnStart };

// Every event carries the HP / status of both sides at that moment. Player one = Charizard, player two = Sceptile.
struct BattleEvent {
    EventType type;
    bool actorisplayerone;
    bool targetisplayerone;
    Type movetype;
    int amount;             // damage dealt; for TurnStart: 1 = attack phase, 0 = move selection
    float effectiveness;
    bool critical;
    int playeronehealth, playertwohealth;
    StatusType playeronestatus, playertwostatus;
    string text;
    string movename;
    BattleEvent() : type(EventType::Log), actorisplayerone(true), targetisplayerone(false),
        movetype(Type::Normal), amount(0), effectiveness(1.f), critical(false), playeronehealth(0), playertwohealth(0),
        playeronestatus(StatusType::None), playertwostatus(StatusType::None), text(), movename() {}
};

struct TurnAction {
    int bracket;
    int speed;
    int tiebreak;
    bool isplayerone;
    int moveindex;
    TurnAction() : bracket(0), speed(0), tiebreak(0), isplayerone(true), moveindex(0) {}
};

// Max-heap order: higher bracket first, then higher speed, then random tie-break.
struct TurnActionLess {
    bool operator()(const TurnAction& first, const TurnAction& second) const {
        if (first.bracket != second.bracket) return first.bracket < second.bracket;
        if (first.speed != second.speed) return first.speed < second.speed;
        return first.tiebreak < second.tiebreak;
    }
};

struct PromptState {
    PokemonState playerone, playertwo;
    int turn;
    PromptState() : turn(1) {}
};

struct TurnRecord {
    PromptState before;
    int playeronemove, playertwomove;
    uint32_t seed;
    TurnRecord() : playeronemove(0), playertwomove(0), seed(1) {}
};

// One round: player one picks a move, player two picks a move, a PriorityQueue orders the two actions
// (bracket, then effective speed, then random tie-break), then the status queue ticks.
class Battle {
    enum class Stage { NotStarted, P1Select, P2Select, Resolving, Over };

    MoveDatabase database;
    Pokemon playeronepokemon, playertwopokemon;
    Stack<TurnRecord> history;
    Queue<BattleEvent> eventqueue;
    Rng masterrng;
    int turn;
    Stage stage;
    bool winnerisplayerone;
    int pendingmove;

    void emit(EventType kind, bool actorisplayerone, bool targetisplayerone, const string& text, int amount = 0,
              float multiplier = 1.f, bool critical = false, Type movetype = Type::Normal,
              const string& movename = string());
    void runround(int playeronemove, int playertwomove, uint32_t seed);
    void performaction(const TurnAction& action, Rng& rng);
    void endofturn();
    bool finishiffainted(bool playeroneside);
    PromptState captureprompt() const;
    void restorefrom(const PromptState& state);

public:
    explicit Battle(uint32_t seed);

    void begin();
    bool submitmove(int moveindex);
    bool undolastturn();
    bool replaylastturn();
    void reset(uint32_t seed);

    const Pokemon& playerone() const { return playeronepokemon; }
    const Pokemon& playertwo() const { return playertwopokemon; }
    const MoveDatabase& movedatabase() const { return database; }
    Queue<BattleEvent>& events() { return eventqueue; }
    bool isover() const { return stage == Stage::Over; }
    bool awaitinginput() const { return stage == Stage::P1Select || stage == Stage::P2Select; }
    int selectingside() const { return stage == Stage::P1Select ? 1 : stage == Stage::P2Select ? 2 : 0; }
    bool playeronewon() const { return winnerisplayerone; }
    int turnnumber() const { return turn; }
    size_t historysize() const { return history.size(); }
};
