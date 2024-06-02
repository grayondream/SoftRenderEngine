#pragma once
#include <cstdint>
#include <utility>
#include <algorithm>
#include "Polar2D.hpp"
#include "Polar3D.hpp"

template<class T>
class Polar3DBase;

template<class T>
class Vector3DBase{
public:
    using ValueType = T;
    constexpr static const std::size_t Size = 3;
public:
    Vector3DBase(const std::initializer_list<T> &ls){
        assert(Size == ls.size());
        this->x = *(ls.begin());
        this->y = *(ls.begin() + 1);
        this->z = *(ls.begin() + 2);
    }

    Vector3DBase(const Vector3DBase &vec){
        std::copy_n(vec.data, Size, data);
    }

    template<class U>
    Vector3DBase(const Polar2DBase<U> &pt){
        z = pt.z;
        x = pt.r * std::cos(pt.thetha);
        y = pt.r * std::sin(pt.thetha);
    }
public:
    template<class U>
    Vector3DBase<ValueType>& operator+=(const Vector3DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    Vector3DBase<ValueType>& operator+=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    Vector3DBase<ValueType>& operator-=(const Vector3DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    Vector3DBase<ValueType>& operator-=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    Vector3DBase<ValueType>& operator*=(const Vector3DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    Vector3DBase<ValueType>& operator*=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    Vector3DBase<ValueType>& operator/=(const Vector3DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 / v2; });
    }

    template<class U>
    Vector3DBase<ValueType>& operator/=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 / v2; });
    }

    bool operator==(const Vector3DBase &vec) const{
        return vec.x == this->x && vec.y == this->y && vec.z == this->z;
    }

public:
    /*
     * @brief 遍历每个元素，针对当前元素和输入的mat同位置的元素同时执行func，返回的值会写入当前矩阵中
     */
    template<class U, typename Func>
    Vector3DBase<ValueType>& foreachFuncBetweenMatrix(const Vector3DBase<U> &mat, Func &&func){
        this->x = func(this->x, mat.x);
        this->y = func(this->y, mat.y);
        this->z = func(this->z, mat.z);
        return *this;
    }

    template<typename Func>
    Vector3DBase<ValueType> &foreachFuncSingleValue(Func &&func){
        this->x = func(this->x);
        this->y = func(this->y);
        this->z = func(this->z);
        return *this;
    }

    template<class U, typename Func>
    Vector3DBase<ValueType> &foreachFuncBinaryValue(const U &u, Func &&func){
        this->x = func(this->x, u);
        this->y = func(this->y, u);
        this->z = func(this->z, u);
        return *this;
    }

    template<class U, typename Func>
    U foreachFuncTotal(Func &&func){
        return this->x + this->y + this->z;
    }
public:
    template<class U>
    U sum(){
        return this->foreachFuncTotal<U>([](const ValueType v1, const ValueType v2){ return v1 + v2; });
    }

    Vector3DBase<ValueType>& fill(const ValueType v = 1 + ValueType{}){
        return this->foreachFuncSingleValue([&v](const ValueType&){ return v; });
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

template<class T>
auto operator+(const Vector3DBase<T> &m1, const Vector3DBase<T> &m2){
    Vector3DBase<T> ret(m1);
    return ret += m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Vector3DBase<T>>>>
auto operator+(const Vector3DBase<T> &m1, const U &val){
    Vector3DBase<T> ret(m1);
    return ret += val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Vector3DBase<T>>>>
auto operator+(const U &val, const Vector3DBase<T> &m1){
    return m1 + val;
}

template<class T>
auto operator-(const Vector3DBase<T> m1, const Vector3DBase<T> &m2){
    Vector3DBase<T> ret(m1);
    return ret -= m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Vector3DBase<T>>>>
auto operator-(const Vector3DBase<T> m1, const U &val){
    Vector3DBase<T> ret(m1);
    return ret -= val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Vector3DBase<T>>>>
auto operator-(const U &val, const Vector3DBase<T> m1){
    return val + ( -1 * m1);
}

template<class T>
auto operator*(const Vector3DBase<T> m1, const Vector3DBase<T> &m2){
    Vector3DBase<T> ret(m1);
    return ret *= m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Vector3DBase<T>>>>
auto operator*(const Vector3DBase<T> m1, const U &val){
    Vector3DBase<T> ret(m1);
    return ret *= val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Vector3DBase<T>>>>
auto operator*(const U &val, const Vector3DBase<T> m1){
    return m1 * val;
}

template<class T>
auto operator/(const Vector3DBase<T> &m1, const Vector3DBase<T> &m2){
    Vector3DBase<T> ret(m1);
    return ret /= m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Vector3DBase<T>>>>
auto operator/(const Vector3DBase<T> &m1,const U &val){
    Vector3DBase<T> ret(m1);
    return ret /= val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Vector3DBase<T>>>>
auto operator/(const U &val, const Vector3DBase<T> &m1){
    using ReturnType = std::common_type_t<T, U>;
    Vector3DBase<ReturnType> ret(m1);
    ret.fill(ReturnType{} + 1);
    ret *= val;
    return ret / m1;
}
