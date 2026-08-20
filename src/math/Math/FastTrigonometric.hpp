#pragma once
#include <cmath>
#include "PreComputeTable.hpp"

namespace Math{

inline static double FastTrigonometric(const double x, const decltype(PrecomputedCosineTable) &table){
    auto angle = std::fmod(x, 360);
    angle = angle < 0 ? angle + 360 : angle;
    int angleInt = angle;
    double angleFloat = angle - angleInt;
    return table[angleInt] + angleFloat * (table[angleInt + 1] - table[angleInt]);
}

inline static double FastCos(const double x){
    return FastTrigonometric(x, PrecomputedCosineTable);
}

inline static double FastSin(const double x){
    return FastTrigonometric(x, PrecomputedSinTable);
}

inline static double FastTan(const double x){
    return FastTrigonometric(x, PrecomputedTanTable);
}
}