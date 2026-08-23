#pragma once
#include "DynamicMatrix2D.hpp"
#include "DynamicMatrixBase.hpp"
#include "DynamicMatrix1DIndex.hpp"
#include "StaticMatrix2D.hpp"
#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

template<class T>
class MatrixIndex1;

template<class T>
class Matrix1DBase : public MatrixBase<T>, public MatrixIndex1<T>{
public:
    using ValueType = typename DynamicMatrixTraits<T>::ValueType;
    using ConstValueType = typename DynamicMatrixTraits<T>::ConstValueType;
    using Pointer = typename DynamicMatrixTraits<T>::Pointer;
    using ConstPointer = typename DynamicMatrixTraits<T>::ConstPointer;
    using Reference = typename DynamicMatrixTraits<T>::Reference;
    using ConstReference = typename DynamicMatrixTraits<T>::ConstReference;
    using MatrixDataType = typename DynamicMatrixTraits<T>::MatrixDataType;

public:
    Matrix1DBase() : MatrixBase<ValueType>(0), MatrixIndex1<ValueType>(this->getRawBuffer(), 0){ }

    Matrix1DBase(ConstMatrixSizeType d1)
        : MatrixBase<ValueType>(d1), MatrixIndex1<ValueType>(this->getRawBuffer(), d1){}

    Matrix1DBase(const std::initializer_list<ValueType> &ls)
        : MatrixBase<ValueType>(ls), MatrixIndex1<ValueType>(this->getRawBuffer(), ls.size()){
        assert(ls.size() > 0);
    }

    Matrix1DBase(ConstPointer data, ConstMatrixSizeType d1)
        : MatrixBase<ValueType>(data, d1), MatrixIndex1<ValueType>(this->getRawBuffer(), d1){}

    Matrix1DBase(const MatrixDataType &vec)
        :MatrixBase<ValueType>(vec), MatrixIndex1<ValueType>(this->getRawBuffer(), vec.size()){}

    Matrix1DBase(const MatrixIndex1<ValueType> &index)
        : MatrixBase<ValueType>(index.m_pstart, index.d1), MatrixIndex1<ValueType>(index){}

    Matrix1DBase(const Matrix1DBase &m1)
        : MatrixBase<ValueType>(m1.m_data), MatrixIndex1<ValueType>(this->getRawBuffer(), m1.d1){}
    
    Matrix1DBase& operator=(const Matrix1DBase &m1){
        if(this == &m1) return *this;

        this->m_data = m1.m_data;
        this->m_pstart = this->getRawBuffer();
        this->d1 = m1.d1;
        return *this;
    }

    Matrix1DBase& operator=(const ValueType &val){
        std::fill_n(this->m_pstart, this->d1, val);
        return *this;
    }
public:
    /*
     * @brief 遍历每个元素，针对当前元素和输入的mat同位置的元素同时执行func，返回的值会写入当前矩阵中
     */
    template<class U, typename Func>
    Matrix1DBase<ValueType>& foreachFuncBetweenMatrix(const Matrix1DBase<U> &mat, Func &&func){
        assert(mat.d1 == this->d1);
        for(std::size_t i = 0;i < this->d1;i ++){
            (*this)[i] = func((*this)[i], mat[i]);
        }

        return *this;
    }

    template<typename Func>
    Matrix1DBase<ValueType> &foreachFuncSingleValue(Func &&func){
        for(std::size_t i = 0; i < this->d1;i ++){
            (*this)[i] = func((*this)[i]);
        }

        return *this;
    }

    template<class U, typename Func>
    Matrix1DBase<ValueType> &foreachFuncBinaryValue(const U &u, Func &&func){
        for(std::size_t i = 0; i < this->d1;i ++){
            (*this)[i] = func((*this)[i], u);
        }
        
        return *this;
    }

    template<class U, typename Func>
    U foreachFuncTotal(Func &&func){
        U u{};
        for(std::size_t i = 0;i < this->d1;i ++){
            u = func(u, (*this)[i]);
        }

        return u;
    }
public:
    //数学计算相关的operator重载
    template<class U>
    Matrix1DBase<ValueType>& operator+=(const Matrix1DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    Matrix1DBase<ValueType>& operator+=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    Matrix1DBase<ValueType>& operator-=(const Matrix1DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    Matrix1DBase<ValueType>& operator-=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    Matrix1DBase<ValueType>& operator*=(const Matrix1DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    Matrix1DBase<ValueType>& operator*=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    Matrix1DBase<ValueType>& operator/=(const Matrix1DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 / v2; });
    }

    template<class U>
    Matrix1DBase<ValueType>& operator/=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 / v2; });
    }

    template<class U>
    Matrix1DBase<ValueType> mul(const Matrix2DBase<U> &mat) const{
        //1 * xd1 X yd2 * yd1
        //[x1 x2 x3]    X   [y1 y11]
        //                  [y2 y22]
        //                  [y3 y33]
        assert(this->d1 == mat.d2);
        Matrix1DBase<ValueType> ret(mat.d1);
        for(std::size_t k = 0;k < ret.d1;k ++){
            ValueType sum {};
            for(std::size_t i = 0;i < this->d1;i ++){
                sum += (*this)[i] * mat[i][k];
            }

            ret[k] = sum;
        }
        
        return ret;
    }

    template<class U, StaticMatrixSizeType di2, StaticMatrixSizeType di1>
    Matrix1DBase<ValueType> mul(const StaticMatrix2DBase<U, di2, di1> &mat) const{
        assert(this->d1 == mat.d2);
        Matrix1DBase<ValueType> ret(mat.d1);
        for(std::size_t k = 0;k < ret.d1;k ++){
            ValueType sum {};
            for(std::size_t i = 0;i < this->d1;i ++){
                sum += (*this)[i] * mat[i][k];
            }

            ret[k] = sum;
        }
        
        return ret;
    }

    double det() const{
        if(this->d1 == 1){
            return (*this)[0];
        }
        return 0;
    }

    Matrix2DBase<ValueType> transpose() const{
        Matrix2DBase<ValueType> ret(this->d1, 1);
        for(std::size_t i = 0;i < ret.d2;i ++){
            ret[i][0] = (*this)[i];
        }

        return ret;
    }
public:
    template<class U>
    U sum(){
        double acc = 0;
        for(std::size_t i = 0; i < this->d1; i++){
            acc += static_cast<double>((*this)[i]);
        }
        return static_cast<U>(acc);
    }

    Matrix1DBase<ValueType>& eye(const ValueType v = 1 + ValueType{}){
        if(this->d1 > 0){
            (*this)[0] = v;
        }
        return *this;
    }

    Matrix1DBase<ValueType>& fill(const ValueType v = 1 + ValueType{}){
        return this->foreachFuncSingleValue([&v](const ValueType&){ return v; });
    }
};

template<class T, class U>
auto operator==(const Matrix1DBase<U> &m1, const Matrix1DBase<T> &m2){
    if(m1.d1 != m2.d1){
        return false;
    }

    for(std::size_t i = 0;i < m1.d1; i ++){
        if(m2[i] != m1[i]){
            return false;
        }
    }

    return true;
}

template<class T, class U>
auto operator+(const Matrix1DBase<U> &m1, const Matrix1DBase<T> &m2){
    assert(m1.d1 == m2.d1);
    Matrix1DBase<std::common_type_t<T, U>> ret(m1);
    return ret += m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix1DBase<T>>>>
auto operator+(const Matrix1DBase<T> &m1, const U &val){
    Matrix1DBase<std::common_type_t<T, U>> ret(m1);
    return ret += val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix1DBase<T>>>>
auto operator+(const U &val, const Matrix1DBase<T> &m1){
    return m1 + val;
}

template<class T, class U>
auto operator-(const Matrix1DBase<T> m1, const Matrix1DBase<U> &m2){
    assert(m1.d1 == m2.d1);
    Matrix1DBase<std::common_type_t<T, U>> ret(m1);
    return ret -= m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix1DBase<T>>>>
auto operator-(const Matrix1DBase<T> m1, const U &val){
    Matrix1DBase<std::common_type_t<T, U>> ret(m1);
    return ret -= val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix1DBase<T>>>>
auto operator-(const U &val, const Matrix1DBase<T> m1){
    return val + ( -1 * m1);
}

template<class T, class U>
auto operator*(const Matrix1DBase<T> m1, const Matrix1DBase<U> &m2){
    assert(m1.d1 == m2.d1);
    Matrix1DBase<std::common_type_t<T, U>> ret(m1);
    return ret *= m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix1DBase<T>>>>
auto operator*(const Matrix1DBase<T> m1, const U &val){
    Matrix1DBase<std::common_type_t<T, U>> ret(m1);
    return ret *= val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix1DBase<T>>>>
auto operator*(const U &val, const Matrix1DBase<T> m1){
    return m1 * val;
}

template<class T, class U>
auto operator/(const Matrix1DBase<T> &m1, const Matrix1DBase<U> &m2){
    assert(m1.d1 == m2.d1);
    Matrix1DBase<std::common_type_t<T, U>> ret(m1);
    return ret /= m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix1DBase<T>>>>
auto operator/(const Matrix1DBase<T> &m1,const U &val){
    Matrix1DBase<std::common_type_t<T, U>> ret(m1);
    return ret /= val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix1DBase<T>>>>
auto operator/(const U &val, const Matrix1DBase<T> &m1){
    using ReturnType = std::common_type_t<T, U>;
    Matrix1DBase<ReturnType> ret(m1);
    ret.fill(ReturnType{} + 1);
    ret *= val;
    return ret / m1;
}

