#pragma once
#include "MathConst.hpp"
#include "Degree.hpp"

namespace Math{
inline static constexpr double CosTaylor(const double x, const std::size_t iter = TaylorIterations){
    double ret = 1.0;
    double sig = -1;
    double lastTerm = 1.0;
    for(int n = 1;n < TaylorIterations - 1;++n){
        lastTerm = lastTerm * x * x / ((2 * n) * (2 * n - 1));
        ret += lastTerm * sig;
        sig *= -1;
    }

    return ret;
}

inline static constexpr double Cos(const int angle){
    return CosTaylor(Degree2Radius(angle));
}
}