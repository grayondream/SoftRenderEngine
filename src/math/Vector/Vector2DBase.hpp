#pragma once

#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <utility>
#include <cassert>
#include <algorithm>
#include "Polar2D.hpp"

template<class T>
class Polar2DBase;

template<class T>
class Vector2DBase{
public:
    using ValueType = T;
    constexpr static const std::size_t Size = 2;
public:
    Vector2DBase(const std::initializer_list<T> &ls){
        assert(Size == ls.size());
        this->x = *(ls.begin());
        this->y = *(ls.begin() + 1);
    }

    Vector2DBase(const Vector2DBase &vec){
        std::copy_n(vec.data, Size, data);
    }

    template<class U>
    Vector2DBase(const Polar2DBase<U> &pt){
        x = pt.r * std::cos(pt.thetha);
        y = pt.r * std::sin(pt.thetha);
    }

public:
    template<class U>
    Vector2DBase<ValueType>& operator+=(const Vector2DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    Vector2DBase<ValueType>& operator+=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    Vector2DBase<ValueType>& operator-=(const Vector2DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    Vector2DBase<ValueType>& operator-=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    Vector2DBase<ValueType>& operator*=(const Vector2DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    Vector2DBase<ValueType>& operator*=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    Vector2DBase<ValueType>& operator/=(const Vector2DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 / v2; });
    }

    template<class U>
    Vector2DBase<ValueType>& operator/=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 / v2; });
    }

    bool operator==(const Vector2DBase &vec) const{
        return vec.x == this->x && vec.y == this->y;
    }

public:
    /*
     * @brief 遍历每个元素，针对当前元素和输入的mat同位置的元素同时执行func，返回的值会写入当前矩阵中
     */
    template<class U, typename Func>
    Vector2DBase<ValueType>& foreachFuncBetweenMatrix(const Vector2DBase<U> &mat, Func &&func){
        this->x = func(x, mat.x);
        this->y = func(y, mat.y);
        return *this;
    }

    template<typename Func>
    Vector2DBase<ValueType> &foreachFuncSingleValue(Func &&func){
        this->x = func(x);
        this->y = func(y);
        return *this;
    }

    template<class U, typename Func>
    Vector2DBase<ValueType> &foreachFuncBinaryValue(const U &u, Func &&func){
        this->x = func(x, u);
        this->y = func(y, u);
        return *this;
    }

    template<class U, typename Func>
    U foreachFuncTotal(Func &&func){
        return this->x + this->y;
    }
public:
    template<class U>
    U sum(){
        return this->foreachFuncTotal<U>([](const ValueType v1, const ValueType v2){ return v1 + v2; });
    }

    Vector2DBase<ValueType>& fill(const ValueType v = 1 + ValueType{}){
        return this->foreachFuncSingleValue([&v](const ValueType&){ return v; });
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

template<class T>
auto operator+(const Vector2DBase<T> &m1, const Vector2DBase<T> &m2){
    Vector2DBase<T> ret(m1);
    return ret += m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Vector2DBase<T>>>>
auto operator+(const Vector2DBase<T> &m1, const U &val){
    Vector2DBase<T> ret(m1);
    return ret += val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Vector2DBase<T>>>>
auto operator+(const U &val, const Vector2DBase<T> &m1){
    return m1 + val;
}

template<class T>
auto operator-(const Vector2DBase<T> m1, const Vector2DBase<T> &m2){
    Vector2DBase<T> ret(m1);
    return ret -= m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Vector2DBase<T>>>>
auto operator-(const Vector2DBase<T> m1, const U &val){
    Vector2DBase<T> ret(m1);
    return ret -= val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Vector2DBase<T>>>>
auto operator-(const U &val, const Vector2DBase<T> m1){
    return val + ( -1 * m1);
}

template<class T>
auto operator*(const Vector2DBase<T> m1, const Vector2DBase<T> &m2){
    Vector2DBase<T> ret(m1);
    return ret *= m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Vector2DBase<T>>>>
auto operator*(const Vector2DBase<T> m1, const U &val){
    Vector2DBase<T> ret(m1);
    return ret *= val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Vector2DBase<T>>>>
auto operator*(const U &val, const Vector2DBase<T> m1){
    return m1 * val;
}

template<class T>
auto operator/(const Vector2DBase<T> &m1, const Vector2DBase<T> &m2){
    Vector2DBase<T> ret(m1);
    return ret /= m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Vector2DBase<T>>>>
auto operator/(const Vector2DBase<T> &m1,const U &val){
    Vector2DBase<T> ret(m1);
    return ret /= val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Vector2DBase<T>>>>
auto operator/(const U &val, const Vector2DBase<T> &m1){
    using ReturnType = std::common_type_t<T, U>;
    Vector2DBase<ReturnType> ret(m1);
    ret.fill(ReturnType{} + 1);
    ret *= val;
    return ret / m1;
}