#pragma once
#include <cstdint>
using namespace std;

// xorshift32, deterministic per seed so Replay reproduces identical results. range() is inclusive, unit() is [0,1).
class Rng {
    uint32_t state;
public:
    explicit Rng(uint32_t seed = 1) : state(seed ? seed : 0x9E3779B9u) {}
    uint32_t next() { state ^= state << 13; state ^= state >> 17; state ^= state << 5; return state; }
    int range(int low, int high) { return low + (int)(next() % (uint32_t)(high - low + 1)); }
    float unit() { return (next() >> 8) / 16777216.0f; }
};
