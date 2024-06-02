#pragma once
#include "Math.hpp"
#include "MathUtil.hpp"
#include "MathConst.hpp"
#include <utility>
#include <array>

namespace Geometry{

template<Math::MathConstIntType... angle>
constexpr auto GenerateCosineTable(std::integer_sequence<Math::MathConstIntType, angle...>){
    return std::array<Math::MathConstType, sizeof...(angle)>{ Math::Cos(angle)... };
}

constexpr auto MakeCosineTable(){
    return GenerateCosineTable(std::make_index_sequence<Math::ConstAnagle360 + 1>{});
}

inline constexpr auto CosineTable = MakeCosineTable();

}