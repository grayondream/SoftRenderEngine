#pragma once
#include <cstdint>

template<class T>
class Vector4DBase{
public:
    using ValueType = T;
    constexpr static const std::size_t Size = 4;

public:
    Vector4DBase<T> operator+(const Vector4DBase<ValueType> &val){
        return { x + val.x, y + val.y, z + val.z, w + val.w };
    }

    Vector4DBase<T> operator-(const Vector4DBase<ValueType> &val){
        return { x - val.x, y - val.y , z - val.z, w - val.w };
    }

    Vector4DBase<T> operator*(const Vector4DBase<ValueType> &val){
        return { x * val.x, y * val.y, z * val.z, w * val.w };
    }

    Vector4DBase<T> operator/(const Vector4DBase<ValueType> &val){
        return { x / val.x, y / val.y, z / val.z, w / val.w };
    }

    Vector4DBase<T>& operator+=(const Vector4DBase<ValueType> &val){
        *this = (*this) + val;
        return *this;
    }

    Vector4DBase<T>& operator-=(const Vector4DBase<ValueType> &val){
        *this = (*this) - val;
        return *this;
    }

    Vector4DBase<T>& operator*=(const Vector4DBase<ValueType> &val){
        *this = (*this) * val;
        return *this;
    }

    Vector4DBase<T>& operator/=(const Vector4DBase<ValueType> &val){
        *this = (*this) / val;
        return *this;
    }
    
public:
    union{
        ValueType data[Size];
        struct{
            ValueType x;
            ValueType y;
            ValueType z;
            ValueType w;
        };
    };
};