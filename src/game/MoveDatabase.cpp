#include "game/MoveDatabase.hpp"

MoveDatabase::MoveDatabase() : table(16) {
    const MoveCategory physical = MoveCategory::Physical, special = MoveCategory::Special;
    table.insert("Flamethrower", Move("Flamethrower", Type::Fire,   special,  90, 100, 0, StatusType::Burn, 30));
    table.insert("Dragon Claw",  Move("Dragon Claw",  Type::Dragon, physical, 80, 100, 0));
    table.insert("Air Slash",    Move("Air Slash",    Type::Flying, special,  75,  95, 0));
    table.insert("Earthquake",   Move("Earthquake",   Type::Ground, physical, 100, 100, 0));
    table.insert("Leaf Blade",    Move("Leaf Blade",    Type::Grass,    physical, 90, 100, 0));
    table.insert("Dragon Pulse",  Move("Dragon Pulse",  Type::Dragon,   special,  85, 100, 0));
    table.insert("Quick Attack",  Move("Quick Attack",  Type::Normal,   physical, 40, 100, 1));
    table.insert("Thunder Punch", Move("Thunder Punch", Type::Electric, physical, 75, 100, 0, StatusType::Paralysis, 30));
}
