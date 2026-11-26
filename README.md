# Charizard vs Sceptile - DSA Battle Simulator (C++17 + SFML 2.6)

## Layout
include/core/   Module 1 - HashTable, PriorityQueue (max-heap), Stack, Queue
include/game/   Module 2 - Pokemon, Move, MoveDatabase, DamageCalculator, StatusQueue, Battle
include/gfx/    Module 3 - UI widgets (HealthBar, Button, FloatingTextPool, LogOverlay), BattleScene
src/            .cpp files (src/game, src/gfx, main.cpp)      tests/test_core.cpp   headless tests
assets/         sprites and optional font (see assets/README.txt)

## Where each DSA is used
HashTable      MoveDatabase: move name -> Move stats (O(1) lookup every turn and in the UI)
PriorityQueue  Battle::runround orders the two chosen actions by priority bracket, then speed (paralysis halves it), then random tie-break; whoever is first acts first
Stack          Battle::history: one TurnRecord per player prompt -> Undo (pop) and Replay (re-run top with same RNG seed)
Queue          StatusQueue (burn/paralysis ticks, FIFO) and the BattleEvent stream engine -> renderer

## Turn flow (two human players, hot-seat)
Charizard (P1) picks a move from its set -> Sceptile (P2) picks from its own set -> both resolve in PriorityQueue
order (move priority, then speed) -> status queue ticks. U cancels P1's pending pick, or rewinds a whole round.

## Build
g++ (no CMake needed):
  g++ -std=c++17 -O2 -Iinclude src/game/*.cpp src/gfx/BattleScene.cpp src/main.cpp -o pokebattle \
      -lsfml-graphics -lsfml-window -lsfml-system
  g++ -std=c++17 -g -fsanitize=address,undefined -Iinclude src/game/*.cpp tests/test_core.cpp -o test_core && ./test_core
CMake: cmake -S . -B build && cmake --build build   (run ./pokebattle from the project root or build dir)
Ubuntu: sudo apt install libsfml-dev      (needs SFML 2.5/2.6, not 3.x)

## Controls
Arrows / WASD / mouse = select, Enter / click / 1-4 = attack, U = undo turn, R = replay last turn, N = new battle, Esc = quit

## Look and feel
The UI is a recreation of the GBA FireRed/LeafGreen battle screen: a 240x160 pixel grid scaled 4x, colours sampled from
the game, two-box command menu (FIGHT / UNDO / REPLAY / NEW), FIGHT submenu with TYPE / PWR / ACC, and HP bars that
drain. Controls: arrows/WASD or mouse, Enter/Z/click = select, Backspace/X/right-click = back, U undo, R replay, N new.

## Sprite credits
assets/sprites/*.png are the FireRed/LeafGreen sprites (Charizard back, Sceptile front) from the PokeAPI sprites repo
(github.com/PokeAPI/sprites). Pokemon and its characters are (c) Nintendo / Game Freak / Creatures Inc.;
used here for a non-commercial university project only - don't redistribute commercially.

Font: Jersey 10 by Sorkin Type (assets/fonts/font.ttf), licensed under the SIL Open Font License 1.1.
