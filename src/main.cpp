#include "gfx/BattleScene.hpp"

int main() {
    BattleScene scene;
    if (!scene.init()) return 1;
    scene.run();
    return 0;
}
