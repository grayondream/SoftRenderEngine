#pragma once

namespace Math{
inline static constexpr double Pow(double x, int exp){
    return (0 == exp) ? 1 : x * Pow(x, exp - 1);
}


}