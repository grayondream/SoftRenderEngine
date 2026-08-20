#pragma once
#include "MathConst.hpp"
#include "Arithmetic.hpp"
#include <cmath>
#include <cstdint>

namespace Math{
//角度归约到 [-180, 180)，保持 constexpr 可用（fmod 非 constexpr）
inline static constexpr double NormalizeAngle(const double angle){
    auto a = angle;
    while(a >= 180) a -= 360;
    while(a < -180) a += 360;
    return a;
}

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

inline static constexpr double Sin(const double angle){
    return SinTaylor(Degree2Radius(NormalizeAngle(angle)));
}

inline static constexpr double Cos(const double angle){
    return CosTaylor(Degree2Radius(NormalizeAngle(angle)));
}

inline static constexpr double Tan(const double angle){
    return Sin(angle) / Cos(angle);
}

}