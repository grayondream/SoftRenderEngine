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
    auto angle = std::fmod(x, 360.0);
    if(angle < 0) angle += 360.0;
    int angleInt = angle;
    //奇点附近（表值过大时插值失真），直接用 std::tan
    if(std::abs(PrecomputedTanTable[angleInt]) > 100 ||
       std::abs(PrecomputedTanTable[angleInt + 1]) > 100){
        return std::tan(x * 3.14159265358979323846 / 180.0);
    }
    return FastTrigonometric(x, PrecomputedTanTable);
}
}