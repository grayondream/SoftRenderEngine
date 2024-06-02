#pragma once
#include "Math.hpp"
#include "MathUtil.hpp"
#include "MathConst.hpp"
#include <utility>
#include <array>

namespace Geometry{

template<typename Func, Math::MathConstIntType... angle>
constexpr auto GeneratePrecomputedTrigonometricTable(Func &&func, std::integer_sequence<Math::MathConstIntType, angle...>){
    return std::array<Math::MathConstType, sizeof...(angle)>{ func(angle)... };
}

inline static constexpr auto MakeCosineTable(){
    return GeneratePrecomputedTrigonometricTable(Math::Cos, std::make_index_sequence<Math::ConstAnagle360 + 1>{});
}

inline static constexpr auto MakeSinTable(){
    return GeneratePrecomputedTrigonometricTable(Math::Sin, std::make_index_sequence<Math::ConstAnagle360 + 1>{});
}

inline static constexpr auto MakeTanTable(){
    return GeneratePrecomputedTrigonometricTable(Math::Tan, std::make_index_sequence<Math::ConstAnagle360 + 1>{});
}

inline constexpr auto PrecomputedCosineTable = MakeCosineTable();
inline constexpr auto PrecomputedSinTable = MakeSinTable();
inline constexpr auto PrecomputedTanTable = MakeTanTable();

}