#include <cassert>
#include <cstdio>
#include "core/Stack.hpp"
#include "core/Queue.hpp"
#include "core/PriorityQueue.hpp"
#include "core/HashTable.hpp"
#include "game/Battle.hpp"
#include "game/DamageCalculator.hpp"
using namespace std;

static void testdatastructures() {
    Stack<int> stack; for (int i = 0; i < 100; ++i) stack.push(i);
    int value; assert(stack.pop(value) && value == 99 && stack.size() == 99);

    Queue<int> queue; for (int i = 0; i < 5; ++i) queue.enqueue(i);
    Queue<int> queuecopy(queue); queue.dequeue(value); assert(value == 0 && queuecopy.size() == 5);

    PriorityQueue<int> priorityqueue; int numbers[] = {5, 1, 9, 3, 7, 9, 2};
    for (int number : numbers) priorityqueue.push(number);
    int previous = 1000; while (priorityqueue.pop(value)) { assert(value <= previous); previous = value; }

    HashTable<int> table(4);
    char key[16];
    for (int i = 0; i < 1000; ++i) { snprintf(key, sizeof key, "k%d", i); table.insert(key, i); }
    assert(table.size() == 1000 && table.loadfactor() <= 0.75);
    assert(table.find("k500") && *table.find("k500") == 500 && !table.find("nope"));
    assert(table.remove("k500") && !table.contains("k500") && table.size() == 999);
    assert(!table.insert("k1", 42) && *table.find("k1") == 42);
}

static void testengine() {
    Pokemon charizard = Pokemon::makecharizard(), sceptile = Pokemon::makesceptile();
    MoveDatabase database;
    assert(database.size() == 8);
    assert(DamageCalculator::totaleffectiveness(*database.get("Flamethrower"), sceptile) == 2.f);
    assert(DamageCalculator::totaleffectiveness(*database.get("Leaf Blade"), charizard) == 0.25f);
    assert(DamageCalculator::totaleffectiveness(*database.get("Thunder Punch"), charizard) == 2.f);
    assert(DamageCalculator::totaleffectiveness(*database.get("Earthquake"), charizard) == 0.f);
    assert(!charizard.caninflict(StatusType::Burn) && sceptile.caninflict(StatusType::Burn));
    sceptile.status.add(StatusType::Burn, 2);
    int damage; StatusType expired;
    sceptile.status.tick(sceptile.maxhealth, damage, expired); assert(damage > 0 && expired == StatusType::None);
    sceptile.status.tick(sceptile.maxhealth, damage, expired); assert(expired == StatusType::Burn && !sceptile.status.hasany());
}

static void testbattleundo() {
    int totalevents = 0, wins = 0, losses = 0;
    for (unsigned seed = 1; seed <= 300; ++seed) {
        Battle battle(seed * 7919u);
        battle.begin();
        assert(battle.selectingside() == 1);
        int guard = 0;
        while (!battle.isover() && guard++ < 200) {
            int playeronemove = (int)((seed + guard) % 4), playertwomove = (int)((seed * 3 + guard) % 4);
            int playeronehealth = battle.playerone().health, playertwohealth = battle.playertwo().health;
            assert(battle.submitmove(playeronemove) && battle.selectingside() == 2);
            assert(battle.undolastturn() && battle.selectingside() == 1);
            assert(battle.playerone().health == playeronehealth && battle.playertwo().health == playertwohealth);
            assert(battle.submitmove(playeronemove) && battle.submitmove(playertwomove));
            int afterplayerone = battle.playerone().health, afterplayertwo = battle.playertwo().health;
            assert(battle.replaylastturn());
            assert(battle.playerone().health == afterplayerone && battle.playertwo().health == afterplayertwo);
            assert(battle.undolastturn());
            assert(battle.playerone().health == playeronehealth && battle.playertwo().health == playertwohealth && !battle.isover() && battle.selectingside() == 1);
            assert(battle.submitmove(playeronemove) && battle.submitmove(playertwomove));
            BattleEvent event; while (battle.events().dequeue(event)) ++totalevents;
        }
        assert(battle.isover());
        (battle.playeronewon() ? wins : losses)++;
    }
    printf("300 simulated battles: P1 wins=%d, P2 wins=%d, events=%d\n", wins, losses, totalevents);
}

int main() {
    testdatastructures();    puts("[ok] DSA structures");
    testengine(); puts("[ok] game engine");
    testbattleundo(); puts("[ok] battle / undo / replay determinism");
    return 0;
}
