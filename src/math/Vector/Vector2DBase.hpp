#pragma once

#include <cstddef>
template<class T>
class Vector2DBase{
public:
    using ValueType = T;
    constexpr static const std::size_t Size = 2;
public:
    Vector2DBase<T> operator+(const Vector2DBase<ValueType> &val){
        return { x + val.x, y + val.y };
    }

    Vector2DBase<T> operator-(const Vector2DBase<ValueType> &val){
        return { x - val.x, y - val.y };
    }

    Vector2DBase<T> operator*(const Vector2DBase<ValueType> &val){
        return { x * val.x, y * val.y };
    }

    Vector2DBase<T> operator/(const Vector2DBase<ValueType> &val){
        return { x / val.x, y / val.y };
    }

    Vector2DBase<T>& operator+=(const Vector2DBase<ValueType> &val){
        *this = (*this) + val;
        return *this;
    }

    Vector2DBase<T>& operator-=(const Vector2DBase<ValueType> &val){
        *this = (*this) - val;
        return *this;
    }

    Vector2DBase<T>& operator*=(const Vector2DBase<ValueType> &val){
        *this = (*this) * val;
        return *this;
    }

    Vector2DBase<T>& operator/=(const Vector2DBase<ValueType> &val){
        *this = (*this) / val;
        return *this;
    }
public:
    union{
        ValueType data[Size]{};
        struct{
            ValueType x;
            ValueType y;
        };
    };
};

namespace Utils{
namespace Vector{
    

}
}