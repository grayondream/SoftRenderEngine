#pragma once
#include <cstdint>

namespace Math{
using MathConstType = double;
using MathConstIntType = std::size_t;

inline static constexpr const MathConstType Pi = 3.14159265358979323846264338327950288;
inline static constexpr const MathConstType Pi_2 = 2 * Pi;
inline static constexpr const MathConstType Pi_3 = 3 * Pi;
inline static constexpr const MathConstType Pi_4 = 4 * Pi;
inline static constexpr const MathConstType Pi_1_2 = Pi / 2;
inline static constexpr const MathConstType Pi_1_4 = Pi / 4;
inline static constexpr const MathConstType Pi_1_8 = Pi / 8;

//定点数
using FPNumber32 = int32_t;
using FPNumber64 = int64_t;

inline static constexpr const FPNumber32 FP32Shift = 16;
inline static constexpr const FPNumber32 FP32Mag = 2 ^ FP32Shift;
inline static constexpr const FPNumber32 FP32DpMask = 0x0000ffff;
inline static constexpr const FPNumber32 FP32WpMask = 0xffff0000;
inline static constexpr const FPNumber32 FP32RoundUpMask = 0x8000;

inline static constexpr const FPNumber64 FP64Shift = 32;
inline static constexpr const FPNumber64 FP64Mag = 2 ^ FP64Shift;
inline static constexpr const FPNumber64 FP64DpMask = 0x00000000ffffffff;
inline static constexpr const FPNumber64 FP64WpMask = 0xffffffff00000000;
inline static constexpr const FPNumber64 FP64RoundUpMask = 0x80000000;

inline static constexpr const MathConstType EpsilonE4 = 1E-4;
inline static constexpr const MathConstType EpsilonE5 = 1E-5;
inline static constexpr const MathConstType EpsilonE6 = 1E-6;
inline static constexpr const MathConstType EpsilonE7 = 1E-7;

inline static constexpr const MathConstIntType ConstAnagle90 = 90;
inline static constexpr const MathConstIntType ConstAnagle180 = 180;
inline static constexpr const MathConstIntType ConstAnagle270 = 270;
inline static constexpr const MathConstIntType ConstAnagle360 = 360;

inline static constexpr std::size_t TaylorIterations = 100;
}