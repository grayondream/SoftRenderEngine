#pragma once
#include <algorithm>
#include <initializer_list>
#include <vector>
#include <cassert>
#include "StaticMatrixIndex3.hpp"

template<class T, StaticMatrixSizeType di3, StaticMatrixSizeType di2, StaticMatrixSizeType di1>
class StaticMatrix3DBase{
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
    constexpr static const StaticMatrixSizeType Size = d3 * d2 * d1;

public:
    StaticMatrix3DBase() = default;
    StaticMatrix3DBase(const std::vector<ValueType> &vec){
        assert(vec.size() == size());
        std::copy_n(std::begin(vec), size(), getRawBuffer());
    }
    
    StaticMatrix3DBase(const std::initializer_list<ValueType> &ls){
        assert(ls.size() == size());
        std::copy_n(std::begin(ls), size(), getRawBuffer());
    }

    StaticMatrix3DBase(Pointer data){
        std::copy_n(data, size(), getRawBuffer());
    }
    
    StaticMatrix3DBase(const StaticMatrixIndex3<ValueType, d3, d2, d1> &mat){
        std::copy_n(mat.m_pdata, size(), getRawBuffer());
    }

    StaticMatrix3DBase(const ConstStaticMatrixIndex3<ValueType, d3, d2, d1> &mat){
        std::copy_n(mat.m_pdata, size(), getRawBuffer());
    }

    StaticMatrix3DBase(const std::initializer_list<std::initializer_list<std::initializer_list<ValueType>>> &ls){
        assert(ls.size() == d3);
        assert(ls.begin()->size() == d2);
        assert(ls.begin()->begin()->size() == d1);
        Pointer p = getRawBuffer();
        for(auto &&c : ls){
            for(auto && r : c){
                std::copy_n(std::begin(r), d1, p);
                p += d1;
            }
        }
    }

    StaticMatrix3DBase(const StaticMatrix3DBase &mat){
        std::copy_n(mat.getRawBuffer(), mat.size(), getRawBuffer());
    }

    StaticMatrix3DBase& operator=(const StaticMatrix3DBase &mat){
        if(this == &mat) return *this;
        std::copy_n(mat.getRawBuffer(), mat.size(), getRawBuffer());
        return *this;
    }

    StaticMatrixSizeType size() const{
        return Size;
    }

    StaticMatrixSizeType size(){
        return Size;
    }

    ConstPointer getRawBuffer() const{
        return (ConstPointer)(m_pdata);
    }

    Pointer getRawBuffer(){
        return reinterpret_cast<Pointer>(m_pdata);
    }

    ConstStaticMatrixIndex2<ValueType, d2, d1> operator[](const StaticMatrixSizeType idx) const{
        return ConstStaticMatrixIndex2<ValueType, d2, d1>(getRawBuffer() + d2 * d1 * idx);
    }

    StaticMatrixIndex2<ValueType, d2, d1> operator[](const StaticMatrixSizeType idx){
        return StaticMatrixIndex2<ValueType, d2, d1>(getRawBuffer() + d2 * d1 * idx);
    }

public:
    template<class U, typename Func>
    StaticMatrix3DBase<ValueType, d3, d2, d1>& foreachFuncBetweenMatrix(const StaticMatrix3DBase<U, d3, d2, d1> &mat, Func &&func){
        assert(mat.d1 > 0 && mat.d1 == this->d1 && mat.d2 == this->d2 && mat.d3 == this->d3);
        for(auto k = 0;k < this->d3; k ++){
            for(auto i = 0;i < this->d2; i ++){
                for(auto j = 0;j < this->d1; j ++){
                    (*this)[k][i][j] = func((*this)[k][i][j], mat[k][i][j]);
                }
            }
        }
        
        return *this;
    }

    template<typename Func>
    StaticMatrix3DBase<ValueType, d3, d2, d1> &foreachFuncSingleValue(Func &&func){
        for(auto k = 0;k < this->d3; k ++){
            for(auto i = 0;i < this->d2; i ++){
                for(auto j = 0;j < this->d1; j ++){
                    (*this)[k][i][j] = func((*this)[k][i][j]);
                }
            }
        }

        return *this;
    }

    template<class U, typename Func>
    StaticMatrix3DBase<ValueType, d3, d2, d1> &foreachFuncBinaryValue(const U &u, Func &&func){
        for(auto k = 0;k < this->d3; k ++){
            for(auto i = 0;i < this->d2; i ++){
                for(auto j = 0;j < this->d1; j ++){
                    (*this)[k][i][j] = func((*this)[k][i][j], u);
                }
            }
        }
        
        return *this;
    }

    template<class U, typename Func>
    U foreachFuncTotal(Func &&func){
        U u{};
        for(auto k = 0;k < this->d3; k ++){
            for(auto i = 0;i < this->d2; i ++){
                for(auto j = 0;j < this->d1; j ++){
                    u = func(u, (*this)[k][i][j]);
                }
            }
        }

        return u;
    }

public:
    template<class U>
    bool operator==(const StaticMatrix3DBase<U, d3, d2, d1> &mat) const{
        if(mat.d1 != this->d1 || mat.d2 != this->d2 || this->d3 != mat.d3){
            return false;
        }

        auto i = 0, j = 0, k = 0;
        for(k = 0;k < this->d3; k ++){
            for(i = 0;i < this->d2;i ++){
                for(j = 0;j < this->d1 && (*this)[k][i][j] == mat[k][i][j];j ++){}
            }
        }
        

        return i == mat.d1 && j == mat.d2 && k == mat.d3;
    }

    template<class U>
    StaticMatrix3DBase<ValueType, d3, d2, d1>& operator+=(const StaticMatrix3DBase<U, d3, d2, d1> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    StaticMatrix3DBase<ValueType, d3, d2, d1>& operator+=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    StaticMatrix3DBase<ValueType, d3, d2, d1>& operator-=(const StaticMatrix3DBase<U, d3, d2, d1> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    StaticMatrix3DBase<ValueType, d3, d2, d1>& operator-=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    StaticMatrix3DBase<ValueType, d3, d2, d1>& operator*=(const StaticMatrix3DBase<U, d3, d2, d1> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    StaticMatrix3DBase<ValueType, d3, d2, d1>& operator*=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    StaticMatrix3DBase<ValueType, d3, d2, d1>& operator/=(const StaticMatrix3DBase<U, d3, d2, d1> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 / v2; });
    }

    template<class U>
    StaticMatrix3DBase<ValueType, d3, d2, d1>& operator/=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 / v2; });
    }

public:
    template<class U>
    U sum(){
        return this->foreachFuncTotal<U>([](const ValueType v1, const ValueType v2){ return v1 + v2; });
    }

    StaticMatrix3DBase<ValueType, d3, d2, d1>& eye(const ValueType v = 1 + ValueType{}){
        auto size = std::min(this->d1, this->d2);
        for(auto j = 0;j < this->d3; j++){
            for(auto i = 0; i < size ;i ++){
                (*this)[j][i][i] = v;
            }
        }

        return *this;
    }

    StaticMatrix3DBase<ValueType, d3, d2, d1>& fill(const ValueType v = 1 + ValueType{}){
        return this->foreachFuncSingleValue([&v](const ValueType&){ return v; });
    }

private:
    ValueType m_pdata[d3][d2][d1]{};
};

template<class T, class U, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
auto operator+(const StaticMatrix3DBase<U, d3, d2, d1> &m1, const StaticMatrix3DBase<T, d3, d2, d1> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1);
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix3DBase<ReturnType, d3, d2, d1> ret(m1);
    return ret += m2;
}

template<class T, class U, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix3DBase<T, d3, d2, d1>>>>
auto operator+(const StaticMatrix3DBase<T, d3, d2, d1> &m1, const U &val){
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix3DBase<ReturnType, d3, d2, d1> ret(m1);
    return ret += val;
}

template<class T, class U, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix3DBase<T, d3, d2, d1>>>>
auto operator+(const U &val, const StaticMatrix3DBase<T, d3, d2, d1> &m1){
    return m1 + val;
}

template<class T, class U, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
auto operator-(const StaticMatrix3DBase<T, d3, d2, d1> m1, const StaticMatrix3DBase<U, d3, d2, d1> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1);
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix3DBase<ReturnType, d3, d2, d1> ret(m1);
    return ret -= m2;
}

template<class T, class U, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix3DBase<T, d3, d2, d1>>>>
auto operator-(const StaticMatrix3DBase<T, d3, d2, d1> m1, const U &val){
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix3DBase<ReturnType, d3, d2, d1> ret(m1);
    return ret -= val;
}

template<class T, class U, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix3DBase<T, d3, d2, d1>>>>
auto operator-(const U &val, const StaticMatrix3DBase<T, d3, d2, d1> m1){
    return val + ( -1 * m1);
}

template<class T, class U, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
auto operator*(const StaticMatrix3DBase<T, d3, d2, d1> m1, const StaticMatrix3DBase<U, d3, d2, d1> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1);
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix3DBase<ReturnType, d3, d2, d1> ret(m1);
    return ret *= m2;
}

template<class T, class U, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix3DBase<T, d3, d2, d1>>>>
auto operator*(const StaticMatrix3DBase<T, d3, d2, d1> m1, const U &val){
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix3DBase<ReturnType, d3, d2, d1> ret(m1);
    return ret *= val;
}

template<class T, class U, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix3DBase<T, d3, d2, d1>>>>
auto operator*(const U &val, const StaticMatrix3DBase<T, d3, d2, d1> m1){
    return m1 * val;
}

template<class T, class U, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
auto operator/(const StaticMatrix3DBase<T, d3, d2, d1> &m1, const StaticMatrix3DBase<U, d3, d2, d1> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1);
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix3DBase<ReturnType, d3, d2, d1> ret(m1);
    return ret /= m2;
}

template<class T, class U, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix3DBase<T, d3, d2, d1>>>>
auto operator/(const StaticMatrix3DBase<T, d3, d2, d1> &m1,const U &val){
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix3DBase<ReturnType, d3, d2, d1> ret(m1);
    return ret /= val;
}

template<class T, class U, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix3DBase<T, d3, d2, d1>>>>
auto operator/(const U &val, const StaticMatrix3DBase<T, d3, d2, d1> &m1){
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix3DBase<ReturnType, d3, d2, d1> ret(m1);
    ret.fill(ReturnType{} + 1);
    ret *= val;
    return ret / m1;
}