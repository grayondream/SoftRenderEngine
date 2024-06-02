#pragma once

#include "MathConst.hpp"
#include "Degree.hpp"
#include <cstdint>

namespace Math{
inline static constexpr std::size_t TaylorIterations = 10;
inline static constexpr double CosTaylor(const double x, const std::size_t iter = TaylorIterations){
    double result = 1.0;
    double term = 1.0;
    bool neg = false;
    for (int i = 2; i <= TaylorIterations * 2; i += 2) {
        term *= x * x / (i * (i - 1));
        result += neg ? -term : term;
        neg = !neg;
    }

    return result;
}

inline static constexpr double Cos(const int angle){
    return CosTaylor(Degree2Radius(angle));
}
}