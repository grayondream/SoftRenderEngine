#pragma once
#include "MathConst.hpp"

namespace Math{
inline static constexpr double Pow(double x, int exp){
    if(exp < 0){
        return 1.0 / Pow(x, -exp);
    }

    return (0 == exp) ? 1 : x * Pow(x, exp - 1);
}

inline constexpr static MathConstType Degree2Radius(const MathConstType angle){
    return Pi * angle / ConstAnagle180;
}

inline constexpr static MathConstType Radiuse2Degree(const MathConstType radius){
    return ConstAnagle180 * radius / Pi;
}

inline constexpr static int64_t Factorial(const int n){
    return (n <= 1) ? 1 : n * Factorial(n - 1);
}

}