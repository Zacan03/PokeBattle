#pragma once
enum class Type { Normal, Fire, Water, Grass, Electric, Flying, Dragon, Ground, None };
enum class StatusType { None, Burn, Paralysis };
enum class MoveCategory { Physical, Special };

constexpr int typecount = 8;

float typematchup(Type attacker, Type defender);
const char* nameoftype(Type type);
const char* nameofstatus(StatusType status);
