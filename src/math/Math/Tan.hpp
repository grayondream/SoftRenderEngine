#pragma once
#include "MathConst.hpp"
#include "Degree.hpp"
#include "Sin.hpp"
#include "Cosine.hpp"

namespace Math{
inline static constexpr double Tan(const int angle){
    return Sin(angle) / Cos(angle);
}
}