#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include "game/Types.hpp"
#include <cmath>
using namespace std;

namespace ui {

inline sf::Color fromhex(unsigned value) { return sf::Color((value >> 16) & 255, (value >> 8) & 255, value & 255); }

inline sf::Color typecolor(Type type) {
    switch (type) {
        case Type::Normal:   return sf::Color(168, 168, 120);
        case Type::Fire:     return sf::Color(240, 128, 48);
        case Type::Water:    return sf::Color(104, 144, 240);
        case Type::Grass:    return sf::Color(120, 200, 80);
        case Type::Electric: return sf::Color(248, 208, 48);
        case Type::Flying:   return sf::Color(168, 144, 240);
        case Type::Dragon:   return sf::Color(112, 56, 248);
        case Type::Ground:   return sf::Color(224, 192, 104);
        default:             return sf::Color::White;
    }
}

class HealthBar {
    float shown, goal;
public:
    HealthBar() : shown(1.f), goal(1.f) {}
    void settarget(float fraction) { goal = max(0.f, min(1.f, fraction)); }
    void snap(float fraction) { settarget(fraction); shown = goal; }
    void update(float seconds) {
        const float step = 0.55f * seconds;
        float difference = goal - shown;
        shown += fabs(difference) <= step ? difference : (difference < 0 ? -step : step);
    }
    float displayed() const { return shown; }
};

}
