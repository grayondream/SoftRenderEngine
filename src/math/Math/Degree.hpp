#pragma once
#include "MathConst.hpp"

namespace Math{
inline constexpr static MathConstType Degree2Radius(const MathConstType angle){
    return Pi * angle / ConstAnagle180;
}

inline constexpr static MathConstType Radiuse2Degree(const MathConstType radius){
    return ConstAnagle180 * radius / Pi;
}
}