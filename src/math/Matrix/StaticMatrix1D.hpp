
#pragma once
#include <algorithm>
#include <initializer_list>
#include <vector>
#include <cassert>
#include "StaticMatrixIndex1.hpp"
#include "StaticMatrixBase.hpp"

template<class T, StaticMatrixSizeType di1>
class StaticMatrix1DBase{
public:
    using ValueType = typename StaticMatrixTraits<T>::ValueType;
    using ConstValueType = typename StaticMatrixTraits<T>::ConstValueType;
    using Pointer = typename StaticMatrixTraits<T>::Pointer;
    using ConstPointer = typename StaticMatrixTraits<T>::ConstPointer;
    using Reference = typename StaticMatrixTraits<T>::Reference;
    using ConstReference = typename StaticMatrixTraits<T>::ConstReference;
 
public:
    constexpr static const StaticMatrixSizeType d1 = di1;
    constexpr static const StaticMatrixSizeType Size = d1;

public:
    StaticMatrix1DBase() = default;
    StaticMatrix1DBase(const std::vector<ValueType> &vec){
        assert(vec.size() == size());
        std::copy_n(std::begin(vec), size(), getRawBuffer());
    }
    
    StaticMatrix1DBase(const std::initializer_list<ValueType> &ls){
        assert(ls.size() == size());
        std::copy_n(std::begin(ls), size(), getRawBuffer());
    }
    
    StaticMatrix1DBase(Pointer data){
        std::copy_n(data, size(), getRawBuffer());
    }
    
    StaticMatrix1DBase(const StaticMatrixIndex1<ValueType, d1> mat){
        std::copy_n(mat.m_pdata, size(), getRawBuffer());
    }

    StaticMatrix1DBase(const ConstStaticMatrixIndex1<ValueType, d1> mat){
        std::copy_n(mat.m_pdata, size(), getRawBuffer());
    }

    StaticMatrix1DBase(const StaticMatrix1DBase &mat){
        std::copy_n(mat.m_pdata, mat.size(), m_pdata);
    }

    StaticMatrix1DBase& operator=(const StaticMatrix1DBase &mat){
        if(this == &mat){
            return *this;
        }

        std::copy_n(mat.m_pdata, mat.size(), m_pdata);
        return *this;
    }

public:
    /*
     * @brief 遍历每个元素，针对当前元素和输入的mat同位置的元素同时执行func，返回的值会写入当前矩阵中
     */
    template<class U, typename Func>
    StaticMatrix1DBase<ValueType, d1>& foreachFuncBetweenMatrix(const StaticMatrix1DBase<U, d1> &mat, Func &&func){
        assert(mat.d1 == this->d1);
        for(StaticMatrixSizeType i = 0;i < this->d1;i ++){
            (*this)[i] = func((*this)[i], mat[i]);
        }

        return *this;
    }

    template<typename Func>
    StaticMatrix1DBase<ValueType, d1> &foreachFuncSingleValue(Func &&func){
        for(StaticMatrixSizeType i = 0; i < this->d1;i ++){
            (*this)[i] = func((*this)[i]);
        }

        return *this;
    }

    template<class U, typename Func>
    StaticMatrix1DBase<ValueType, d1> &foreachFuncBinaryValue(const U &u, Func &&func){
        for(StaticMatrixSizeType i = 0; i < this->d1;i ++){
            (*this)[i] = func((*this)[i], u);
        }
        
        return *this;
    }

    template<class U, typename Func>
    U foreachFuncTotal(Func &&func){
        U u{};
        for(StaticMatrixSizeType i = 0;i < this->d1;i ++){
            u = func(u, (*this)[i]);
        }

        return u;
    }

public:
    StaticMatrixSizeType size() const{
        return Size;
    }

    StaticMatrixSizeType size(){
        return Size;
    }

    Pointer getRawBuffer() const{
        return m_pdata;
    }

    Pointer getRawBuffer(){
        return reinterpret_cast<Pointer>(m_pdata);
    }
    
    ValueType operator[](const StaticMatrixSizeType idx) const{
        return m_pdata[idx];
    }

    Reference operator[](const StaticMatrixSizeType idx){
        return m_pdata[idx];
    }

public:
    //数学计算相关的operator重载
    template<class U>
    bool operator==(const StaticMatrix1DBase<U, d1> &mat) const {
        if(mat.d1 != this->d1){
            return false;
        }

        StaticMatrixSizeType i = 0;
        for(i = 0;i < mat.d1 && (*this)[i] == mat[i]; i ++){}
        return i == mat.d1;
    }

    template<class U>
    StaticMatrix1DBase<ValueType, d1>& operator+=(const StaticMatrix1DBase<U, d1> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    StaticMatrix1DBase<ValueType, d1>& operator+=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    StaticMatrix1DBase<ValueType, d1>& operator-=(const StaticMatrix1DBase<U, d1> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    StaticMatrix1DBase<ValueType, d1>& operator-=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    StaticMatrix1DBase<ValueType, d1>& operator*=(const StaticMatrix1DBase<U, d1> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    StaticMatrix1DBase<ValueType, d1>& operator*=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    StaticMatrix1DBase<ValueType, d1>& operator/=(const StaticMatrix1DBase<U, d1> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 / v2; });
    }

    template<class U>
    StaticMatrix1DBase<ValueType, d1>& operator/=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 / v2; });
    }

public:
    template<class U>
    U sum(){
        double acc = 0;
        for(StaticMatrixSizeType i = 0; i < d1; i++){
            acc += static_cast<double>((*this)[i]);
        }
        return static_cast<U>(acc);
    }

    StaticMatrix1DBase<ValueType, d1>& eye(const ValueType v = 1 + ValueType{}){
        (*this)[0] = v;
        return *this;
    }

    StaticMatrix1DBase<ValueType, d1>& fill(const ValueType v = 1 + ValueType{}){
        return this->foreachFuncSingleValue([&v](const ValueType&){ return v; });
    }

private:
    ValueType m_pdata[d1]{};
};

template<class T, class U, StaticMatrixSizeType d1>
auto operator+(const StaticMatrix1DBase<U, d1> &m1, const StaticMatrix1DBase<T, d1> &m2){
    assert(m1.d1 == m2.d1);
    StaticMatrix1DBase<std::common_type_t<T, U>, d1> ret(m1);
    return ret += m2;
}

template<class T, class U, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix1DBase<T, d1>>>>
auto operator+(const StaticMatrix1DBase<T, d1> &m1, const U &val){
    StaticMatrix1DBase<std::common_type_t<T, U>, d1> ret(m1);
    return ret += val;
}

template<class T, class U, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix1DBase<T, d1>>>>
auto operator+(const U &val, const StaticMatrix1DBase<T, d1> &m1){
    return m1 + val;
}

template<class T, class U, StaticMatrixSizeType d1>
auto operator-(const StaticMatrix1DBase<T, d1> m1, const StaticMatrix1DBase<U, d1> &m2){
    assert(m1.d1 == m2.d1);
    StaticMatrix1DBase<std::common_type_t<T, U>, d1> ret(m1);
    return ret -= m2;
}

template<class T, class U, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix1DBase<T, d1>>>>
auto operator-(const StaticMatrix1DBase<T, d1> m1, const U &val){
    StaticMatrix1DBase<std::common_type_t<T, U>, d1> ret(m1);
    return ret -= val;
}

template<class T, class U, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix1DBase<T, d1>>>>
auto operator-(const U &val, const StaticMatrix1DBase<T, d1> m1){
    return val + ( -1 * m1);
}

template<class T, class U, StaticMatrixSizeType d1>
auto operator*(const StaticMatrix1DBase<T, d1> m1, const StaticMatrix1DBase<U, d1> &m2){
    assert(m1.d1 == m2.d1);
    StaticMatrix1DBase<std::common_type_t<T, U>, d1> ret(m1);
    return ret *= m2;
}

template<class T, class U, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix1DBase<T, d1>>>>
auto operator*(const StaticMatrix1DBase<T, d1> m1, const U &val){
    StaticMatrix1DBase<std::common_type_t<T, U>, d1> ret(m1);
    return ret *= val;
}

template<class T, class U, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix1DBase<T, d1>>>>
auto operator*(const U &val, const StaticMatrix1DBase<T, d1> m1){
    return m1 * val;
}

template<class T, class U, StaticMatrixSizeType d1>
auto operator/(const StaticMatrix1DBase<T, d1> &m1, const StaticMatrix1DBase<U, d1> &m2){
    assert(m1.d1 == m2.d1);
    StaticMatrix1DBase<std::common_type_t<T, U>, d1> ret(m1);
    return ret /= m2;
}

template<class T, class U, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix1DBase<T, d1>>>>
auto operator/(const StaticMatrix1DBase<T, d1> &m1,const U &val){
    StaticMatrix1DBase<std::common_type_t<T, U>, d1> ret(m1);
    return ret /= val;
}

template<class T, class U, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix1DBase<T, d1>>>>
auto operator/(const U &val, const StaticMatrix1DBase<T, d1> &m1){
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix1DBase<ReturnType, d1> ret(m1);
    ret.fill(ReturnType{} + 1);
    ret *= val;
    return ret / m1;
}