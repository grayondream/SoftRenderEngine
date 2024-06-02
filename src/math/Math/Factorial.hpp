#pragma once


#include <cstdint>
namespace Math{
    inline constexpr static int64_t Factorial(const int n){
        return (n <= 1) ? 1 : n * Factorial(n - 1);
    }
}