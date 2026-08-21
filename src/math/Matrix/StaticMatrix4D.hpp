#pragma once
#include <algorithm>
#include <initializer_list>
#include <vector>
#include <cassert>
#include "StaticMatrixIndex4.hpp"

template<class T, StaticMatrixSizeType di4, StaticMatrixSizeType di3, StaticMatrixSizeType di2, StaticMatrixSizeType di1>
class StaticMatrix4DBase{
public:
    using ValueType = typename StaticMatrixTraits<T>::ValueType;
    using ConstValueType = typename StaticMatrixTraits<T>::ConstValueType;
    using Pointer = typename StaticMatrixTraits<T>::Pointer;
    using ConstPointer = typename StaticMatrixTraits<T>::ConstPointer;
    using Reference = typename StaticMatrixTraits<T>::Reference;
    using ConstReference = typename StaticMatrixTraits<T>::ConstReference;
 
public:
    constexpr static const StaticMatrixSizeType d1 = di1;
    constexpr static const StaticMatrixSizeType d2 = di2;
    constexpr static const StaticMatrixSizeType d3 = di3;
    constexpr static const StaticMatrixSizeType d4 = di4;
    constexpr static const StaticMatrixSizeType Size = d4 * d3 * d2 * d1;

public:
    StaticMatrix4DBase() = default;
    StaticMatrix4DBase(const std::vector<ValueType> &vec){
        assert(vec.size() == size());
        std::copy_n(std::begin(vec), size(), getRawBuffer());
    }
    
    StaticMatrix4DBase(const std::initializer_list<ValueType> &ls){
        assert(ls.size() == size());
        std::copy_n(std::begin(ls), size(), getRawBuffer());
    }

    StaticMatrix4DBase(Pointer data){
        std::copy_n(data, size(), getRawBuffer());
    }

    StaticMatrix4DBase(const StaticMatrixIndex4<ValueType, d4, d3, d2, d1> &mat){
        std::copy_n(mat.m_pdata, size(), m_pdata);
    }
    
    StaticMatrix4DBase(const ConstStaticMatrixIndex4<ValueType, d4, d3, d2, d1> &mat){
        std::copy_n(mat.m_pdata, size(), m_pdata);
    }

    StaticMatrix4DBase(const std::initializer_list<std::initializer_list<std::initializer_list<std::initializer_list<ValueType>>>> &ls){
        assert(ls.size() == d4);
        assert(ls.begin()->size() == d3);
        assert(ls.begin()->begin()->size() == d2);
        assert(ls.begin()->begin()->begin()->size() == d1);
        Pointer p = getRawBuffer();
        for(auto && c : ls){
            for(auto &&r : c){
                for(auto && l : r){
                    std::copy_n(std::begin(l), d1, p);
                    p += d1;
                }
            }
        }
    }

    StaticMatrix4DBase(const StaticMatrix4DBase &mat){
        std::copy_n(mat.getRawBuffer(), mat.size(), getRawBuffer());
    }

    StaticMatrix4DBase& operator=(const StaticMatrix4DBase &mat){
        if(this == &mat) return *this;
        std::copy_n(mat.getRawBuffer(), mat.size(), getRawBuffer());
        return *this;
    }
public:

    StaticMatrixSizeType size() const{
        return Size;
    }

    StaticMatrixSizeType size(){
        return Size;
    }

    ConstPointer getRawBuffer() const{
        return reinterpret_cast<ConstPointer>(m_pdata);
    }

    Pointer getRawBuffer(){
        return reinterpret_cast<Pointer>(m_pdata);
    }

    StaticMatrixIndex3<ValueType, d3, d2, d1> operator[](const StaticMatrixSizeType idx) {
        return StaticMatrixIndex3<ValueType, d3, d2, d1>(getRawBuffer() + d3 * d2 * d1 * idx);
    }

    ConstStaticMatrixIndex3<ValueType, d3, d2, d1> operator[](const StaticMatrixSizeType idx) const {
        return ConstStaticMatrixIndex3<ValueType, d3, d2, d1>(getRawBuffer() + d3 * d2 * d1 * idx);
    }

public:
    template<class U, typename Func>
    StaticMatrix4DBase<ValueType, d4, d3, d2, d1>& foreachFuncBetweenMatrix(const StaticMatrix4DBase<U, d4, d3, d2, d1> &mat, Func &&func){
        assert(mat.d1 > 0 && mat.d1 == this->d1 && mat.d2 == this->d2 && mat.d3 == this->d3 && mat.d4 == this->d4);
        for(auto c = 0; c < this->d4; c ++){
            for(auto k = 0;k < this->d3; k ++){
                for(auto i = 0;i < this->d2; i ++){
                    for(auto j = 0;j < this->d1; j ++){
                        (*this)[c][k][i][j] = func((*this)[c][k][i][j], mat[c][k][i][j]);
                    }
                }
            }
        }
        
        return *this;
    }

    template<typename Func>
    StaticMatrix4DBase<ValueType, d4, d3, d2, d1> &foreachFuncSingleValue(Func &&func){
        for(auto c = 0; c < this->d4; c ++){
            for(auto k = 0;k < this->d3; k ++){
                for(auto i = 0;i < this->d2; i ++){
                    for(auto j = 0;j < this->d1; j ++){
                        (*this)[c][k][i][j] = func((*this)[c][k][i][j]);
                    }
                }
            }
        }

        return *this;
    }

    template<class U, typename Func>
    StaticMatrix4DBase<ValueType, d4, d3, d2, d1> &foreachFuncBinaryValue(const U &u, Func &&func){
        for(auto c = 0; c < this->d4; c ++){
            for(auto k = 0;k < this->d3; k ++){
                for(auto i = 0;i < this->d2; i ++){
                    for(auto j = 0;j < this->d1; j ++){
                        (*this)[c][k][i][j] = func((*this)[c][k][i][j], u);
                    }
                }
            }
        }
        
        return *this;
    }

    template<class U, typename Func>
    U foreachFuncTotal(Func &&func){
        U u{};
        for(auto c = 0; c < this->d4; c ++){
            for(auto k = 0;k < this->d3; k ++){
                for(auto i = 0;i < this->d2; i ++){
                    for(auto j = 0;j < this->d1; j ++){
                        u = func(u, (*this)[c][k][i][j]);
                    }
                }
            }
        }

        return u;
    }

public:
    template<class U>
    bool operator==(const StaticMatrix4DBase<U, d4, d3, d2, d1> &mat) const{
        if(mat.d1 != this->d1 || mat.d2 != this->d2 || this->d3 != mat.d3 || mat.d4 != this->d4){
            return false;
        }

        for(auto c = 0; c < this->d4; c ++){
            for(auto k = 0;k < this->d3; k ++){
                for(auto i = 0;i < this->d2;i ++){
                    for(auto j = 0;j < this->d1;j ++){
                        if((*this)[c][k][i][j] != mat[c][k][i][j]){
                            return false;
                        }
                    }
                }
            }
        }    

        return true;
    }

    template<class U>
    StaticMatrix4DBase<ValueType, d4, d3, d2, d1>& operator+=(const StaticMatrix4DBase<U, d4, d3, d2, d1> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    StaticMatrix4DBase<ValueType, d4, d3, d2, d1>& operator+=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    StaticMatrix4DBase<ValueType, d4, d3, d2, d1>& operator-=(const StaticMatrix4DBase<U, d4, d3, d2, d1> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    StaticMatrix4DBase<ValueType, d4, d3, d2, d1>& operator-=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    StaticMatrix4DBase<ValueType, d4, d3, d2, d1>& operator*=(const StaticMatrix4DBase<U, d4, d3, d2, d1> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    StaticMatrix4DBase<ValueType, d4, d3, d2, d1>& operator*=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    StaticMatrix4DBase<ValueType, d4, d3, d2, d1>& operator/=(const StaticMatrix4DBase<U, d4, d3, d2, d1> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 / v2; });
    }

    template<class U>
    StaticMatrix4DBase<ValueType, d4, d3, d2, d1>& operator/=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 / v2; });
    }

public:
    template<class U>
    U sum(){
        double acc = 0;
        for(StaticMatrixSizeType c = 0; c < d4; c++){
            for(StaticMatrixSizeType k = 0; k < d3; k++){
                for(StaticMatrixSizeType i = 0; i < d2; i++){
                    for(StaticMatrixSizeType j = 0; j < d1; j++){
                        acc += static_cast<double>((*this)[c][k][i][j]);
                    }
                }
            }
        }
        return static_cast<U>(acc);
    }

    StaticMatrix4DBase<ValueType, d4, d3, d2, d1>& eye(const ValueType v = 1 + ValueType{}){
        this->fill(ValueType{});
        auto size = std::min(std::min(this->d1, this->d2), this->d3);
        for(auto c = 0;c < this->d4; c++){
            for(auto j = 0;j < this->d3; j++){
                for(auto i = 0; i < size ;i ++){
                    (*this)[c][j][i][i] = v;
                }
            }
        }
        
        return *this;
    }

    StaticMatrix4DBase<ValueType, d4, d3, d2, d1>& fill(const ValueType v = 1 + ValueType{}){
        return this->foreachFuncSingleValue([&v](const ValueType&){ return v; });
    }

    template<class U, StaticMatrixSizeType od4, StaticMatrixSizeType od3, StaticMatrixSizeType od2, StaticMatrixSizeType od1>
    StaticMatrix4DBase<ValueType, d4, d3, d2, od1> mul(
        const StaticMatrix4DBase<U, od4, od3, od2, od1> &mat) const{
        static_assert(d1 == od2, "Matrix dimensions must match for multiplication");
        StaticMatrix4DBase<ValueType, d4, d3, d2, od1> ret{};
        for(auto c = 0; c < d4; c++){
            for(auto k = 0; k < d3; k++){
                for(auto i = 0; i < d2; i++){
                    for(auto j = 0; j < od1; j++){
                        ValueType sum{};
                        for(auto s = 0; s < d1; s++){
                            sum += (*this)[c][k][i][s] * mat[c][k][s][j];
                        }
                        ret[c][k][i][j] = sum;
                    }
                }
            }
        }
        return ret;
    }

    template<class U>
    Vector4DBase<ValueType> mul(const Vector4DBase<U> &vec) const{
        static_assert(d2 == 4 && d1 == 4, "Matrix must be 4x4 for vector multiplication");
        return Vector4DBase<ValueType>(
            (*this)[0][0][0][0]*vec.x + (*this)[0][0][0][1]*vec.y + (*this)[0][0][0][2]*vec.z + (*this)[0][0][0][3]*vec.w,
            (*this)[0][0][1][0]*vec.x + (*this)[0][0][1][1]*vec.y + (*this)[0][0][1][2]*vec.z + (*this)[0][0][1][3]*vec.w,
            (*this)[0][0][2][0]*vec.x + (*this)[0][0][2][1]*vec.y + (*this)[0][0][2][2]*vec.z + (*this)[0][0][2][3]*vec.w,
            (*this)[0][0][3][0]*vec.x + (*this)[0][0][3][1]*vec.y + (*this)[0][0][3][2]*vec.z + (*this)[0][0][3][3]*vec.w
        );
    }

private:
    ValueType m_pdata[d4][d3][d2][d1]{};
};

template<class T, class U, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
auto operator+(const StaticMatrix4DBase<U, d4, d3, d2, d1> &m1, const StaticMatrix4DBase<T, d4, d3, d2, d1> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1 && m1.d2 == m2.d2 && m1.d3 == m2.d3 && m1.d4 == m2.d4);
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix4DBase<ReturnType, d4, d3, d2, d1> ret(m1);
    return ret += m2;
}

template<class T, class U, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix4DBase<T, d4, d3, d2, d1>>>>
auto operator+(const StaticMatrix4DBase<T, d4, d3, d2, d1> &m1, const U &val){
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix4DBase<ReturnType, d4, d3, d2, d1> ret(m1);
    return ret += val;
}

template<class T, class U, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix4DBase<T, d4, d3, d2, d1>>>>
auto operator+(const U &val, const StaticMatrix4DBase<T, d4, d3, d2, d1> &m1){
    return m1 + val;
}

template<class T, class U, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
auto operator-(const StaticMatrix4DBase<T, d4, d3, d2, d1> m1, const StaticMatrix4DBase<U, d4, d3, d2, d1> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1 && m1.d2 == m2.d2 && m1.d3 == m2.d3 && m1.d4 == m2.d4);
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix4DBase<ReturnType, d4, d3, d2, d1> ret(m1);
    return ret -= m2;
}

template<class T, class U, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix4DBase<T, d4, d3, d2, d1>>>>
auto operator-(const StaticMatrix4DBase<T, d4, d3, d2, d1> m1, const U &val){
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix4DBase<ReturnType, d4, d3, d2, d1> ret(m1);
    return ret -= val;
}

template<class T, class U, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix4DBase<T, d4, d3, d2, d1>>>>
auto operator-(const U &val, const StaticMatrix4DBase<T, d4, d3, d2, d1> m1){
    return val + ( -1 * m1);
}

template<class T, class U, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
auto operator*(const StaticMatrix4DBase<T, d4, d3, d2, d1> m1, const StaticMatrix4DBase<U, d4, d3, d2, d1> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1 && m1.d2 == m2.d2 && m1.d3 == m2.d3 && m1.d4 == m2.d4);
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix4DBase<ReturnType, d4, d3, d2, d1> ret(m1);
    return ret *= m2;
}

template<class T, class U, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix4DBase<T, d4, d3, d2, d1>>>>
auto operator*(const StaticMatrix4DBase<T, d4, d3, d2, d1> m1, const U &val){
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix4DBase<ReturnType, d4, d3, d2, d1> ret(m1);
    return ret *= val;
}

template<class T, class U, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix4DBase<T, d4, d3, d2, d1>>>>
auto operator*(const U &val, const StaticMatrix4DBase<T, d4, d3, d2, d1> m1){
    return m1 * val;
}

template<class T, class U, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
auto operator/(const StaticMatrix4DBase<T, d4, d3, d2, d1> &m1, const StaticMatrix4DBase<U, d4, d3, d2, d1> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1 && m1.d2 == m2.d2 && m1.d3 == m2.d3 && m1.d4 == m2.d4);
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix4DBase<ReturnType, d4, d3, d2, d1> ret(m1);
    return ret /= m2;
}

template<class T, class U, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix4DBase<T, d4, d3, d2, d1>>>>
auto operator/(const StaticMatrix4DBase<T, d4, d3, d2, d1> &m1,const U &val){
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix4DBase<ReturnType, d4, d3, d2, d1> ret(m1);
    return ret /= val;
}

template<class T, class U, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix4DBase<T, d4, d3, d2, d1>>>>
auto operator/(const U &val, const StaticMatrix4DBase<T, d4, d3, d2, d1> &m1){
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix4DBase<ReturnType, d4, d3, d2, d1> ret(m1);
    ret.fill(ReturnType{} + 1);
    ret *= val;
    return ret / m1;
}
