#pragma once
#include <cstdint>

template<class T>
class Vector3DBase{
public:
    using ValueType = T;
    constexpr static const std::size_t Size = 2;
public:
    Vector3DBase<T> operator+(const Vector3DBase<ValueType> &val){
        return { x + val.x, y + val.y, z + val.z };
    }

    Vector3DBase<T> operator-(const Vector3DBase<ValueType> &val){
        return { x - val.x, y - val.y , z - val.z };
    }

    Vector3DBase<T> operator*(const Vector3DBase<ValueType> &val){
        return { x * val.x, y * val.y, z * val.z };
    }

    Vector3DBase<T> operator/(const Vector3DBase<ValueType> &val){
        return { x / val.x, y / val.y, z / val.z };
    }

    Vector3DBase<T>& operator+=(const Vector3DBase<ValueType> &val){
        *this = (*this) + val;
        return *this;
    }

    Vector3DBase<T>& operator-=(const Vector3DBase<ValueType> &val){
        *this = (*this) - val;
        return *this;
    }

    Vector3DBase<T>& operator*=(const Vector3DBase<ValueType> &val){
        *this = (*this) * val;
        return *this;
    }

    Vector3DBase<T>& operator/=(const Vector3DBase<ValueType> &val){
        *this = (*this) / val;
        return *this;
    }
    
public:
    union{
        ValueType data[Size]{};
        struct{
            ValueType x;
            ValueType y;
            ValueType z;
        };
    };
};
