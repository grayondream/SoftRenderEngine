#pragma once
#include "MathConst.hpp"
#include "Degree.hpp"

namespace Math{
inline static constexpr double SinTaylor(const double x, const std::size_t iter = TaylorIterations){
    double ret = x;
    double sig = -1;
    double lastTerm = x;
    for(int n = 1;n < TaylorIterations;++n){
        lastTerm = lastTerm * x * x / ((2 * n + 1) * (2 * n));
        ret += lastTerm * sig;
        sig *= -1;
    }

    return ret;
}

inline static constexpr double Sin(const int angle){
    return SinTaylor(Degree2Radius(angle));
}
}