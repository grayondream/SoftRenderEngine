#pragma once
#include "Trigonometric.hpp"
#include "MathUtil.hpp"
#include "MathConst.hpp"
#include <utility>
#include <array>

namespace Math{

template<typename Func,  MathConstIntType... angle>
constexpr auto GeneratePrecomputedTrigonometricTable(Func &&func, std::integer_sequence< MathConstIntType, angle...>){
    return std::array< MathConstType, sizeof...(angle)>{ func(angle)... };
}

inline static constexpr auto MakeCosineTable(){
    return GeneratePrecomputedTrigonometricTable( Cos, std::make_index_sequence< ConstAnagle360 + 1>{});
}

inline static constexpr auto MakeSinTable(){
    return GeneratePrecomputedTrigonometricTable( Sin, std::make_index_sequence< ConstAnagle360 + 1>{});
}

inline static constexpr auto MakeTanTable(){
    return GeneratePrecomputedTrigonometricTable( Tan, std::make_index_sequence< ConstAnagle360 + 1>{});
}

inline constexpr auto PrecomputedCosineTable = MakeCosineTable();
inline constexpr auto PrecomputedSinTable = MakeSinTable();
inline constexpr auto PrecomputedTanTable = MakeTanTable();

}