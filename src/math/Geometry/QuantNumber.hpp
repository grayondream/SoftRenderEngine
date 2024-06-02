//四元数
#pragma once
#include <cstdint>
#include <utility>
#include <algorithm>
#include "Vector3DBase.hpp"
template<class T>
class QuantNumber{
public:
    using ValueType = T;
    constexpr static const std::size_t Size = 4;

public:
    QuantNumber(const QuantNumber &number){
        std::copy_n(number.data, Size, data);
    }

public:
    union{
        ValueType data[Size]{};
        struct{
            ValueType w;
            ValueType x;
            ValueType y;
            ValueType z;
        };
        struct{
            T q0;                   //实部
            Vector3DBase<T> qv;     //三虚部
        };
    };
};

