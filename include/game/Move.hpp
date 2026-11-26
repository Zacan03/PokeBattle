#pragma once
#include <string>
#include "game/Types.hpp"
using namespace std;

struct Move {
    string name;
    Type type;
    MoveCategory category;
    int power;
    int accuracy;
    int priority;
    StatusType status;
    int statuschance;
    Move() : name(), type(Type::Normal), category(MoveCategory::Physical), power(0),
             accuracy(100), priority(0), status(StatusType::None), statuschance(0) {}
    Move(const string& givenname, Type giventype, MoveCategory givencategory, int givenpower, int givenaccuracy,
         int givenpriority, StatusType givenstatus = StatusType::None, int givenchance = 0)
        : name(givenname), type(giventype), category(givencategory), power(givenpower),
          accuracy(givenaccuracy), priority(givenpriority), status(givenstatus), statuschance(givenchance) {}
};
