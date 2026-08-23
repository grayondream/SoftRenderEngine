#pragma once
#include "DynamicMatrixBase.hpp"
#include "DynamicMatrix3DIndex.hpp"
#include "Vector3DBase.hpp"

template<class T> class Matrix3DBase : public MatrixBase<T>, public MatrixIndex3<T>{
public:
    using ValueType = DynamicMatrixTraits<T>::ValueType;
    using ConstValueType = DynamicMatrixTraits<T>::ConstValueType;
    using Pointer = DynamicMatrixTraits<T>::Pointer;
    using ConstPointer = DynamicMatrixTraits<T>::ConstPointer;
    using Reference = DynamicMatrixTraits<T>::Reference;
    using ConstReference = DynamicMatrixTraits<T>::ConstReference;
    using MatrixDataType = DynamicMatrixTraits<T>::MatrixDataType;

public:
    Matrix3DBase() : MatrixBase<ValueType>(0), MatrixIndex3<ValueType>(this->getRawBuffer(), 0, 0, 0){ }

    Matrix3DBase(ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1)
    : MatrixBase<ValueType>(d1 * d2 * d3), MatrixIndex3<ValueType>(this->getRawBuffer(), d3, d2, d1){}

    Matrix3DBase(const std::initializer_list<ValueType> &ls, ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1)
        : MatrixBase<ValueType>(ls), MatrixIndex3<ValueType>(this->getRawBuffer(), d3, d2, d1){
        assert(ls.size() > 0);
        assert(d3 * d2 * d1 == ls.size());
    }

    Matrix3DBase(ConstPointer data, ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1)
        : MatrixBase<ValueType>(data, d3 * d2 * d1), MatrixIndex3<ValueType>(this->getRawBuffer(), d3, d2, d1){}

    Matrix3DBase(const MatrixDataType &vec, ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1)
        : MatrixBase<ValueType>(vec), MatrixIndex3<ValueType>(this->getRawBuffer(), d3, d2, d1){}

    Matrix3DBase(const MatrixIndex3<ValueType> &index)
        : MatrixBase<ValueType>(index.m_pstart, index.d1 * index.d2 * index.d3), MatrixIndex3<ValueType>(index){}

    Matrix3DBase(const std::initializer_list<std::initializer_list<std::initializer_list<ValueType>>> &ls)
        :   MatrixBase<ValueType>(ls.size() * ls.begin()->size() * ls.begin()->begin()->size()),
            MatrixIndex3<ValueType>(this->getRawBuffer(), ls.size(), ls.begin()->size(), ls.begin()->begin()->size()){
        assert(ls.size() > 0);
        assert(ls.begin()->size() == this->d2 && ls.begin()->begin()->size() == this->d1);
        Pointer p = this->getRawBuffer();
        for(auto &&c : ls){
            assert(c.size() == this->d2);
            for(auto && r : c){
                assert(r.size() == this->d1);
                std::copy_n(std::begin(r), this->d1, p);
                p += this->d1;
            }
        }
    }

    Matrix3DBase(const Matrix3DBase &mat)
    :   MatrixBase<ValueType>(mat.m_data), 
        MatrixIndex3<ValueType>(this->getRawBuffer(), mat.d3, mat.d2, mat.d1){}

    Matrix3DBase& operator=(const Matrix3DBase &mat){
        if(this == &mat) return *this;
        this->m_data = mat.m_data;
        this->m_pstart = this->getRawBuffer();
        this->d1 = mat.d1;
        this->d2 = mat.d2;
        this->d3 = mat.d3;
        return *this;
    }

public:
    template<class U, typename Func>
    Matrix3DBase<ValueType>& foreachFuncBetweenMatrix(const Matrix3DBase<U> &mat, Func &&func){
        assert(mat.d1 > 0 && mat.d1 == this->d1 && mat.d2 == this->d2 && mat.d3 == this->d3);
        for(std::size_t k = 0;k < this->d3; k ++){
            for(std::size_t i = 0;i < this->d2; i ++){
                for(std::size_t j = 0;j < this->d1; j ++){
                    (*this)[k][i][j] = func((*this)[k][i][j], mat[k][i][j]);
                }
            }
        }
        
        return *this;
    }

    template<typename Func>
    Matrix3DBase<ValueType> &foreachFuncSingleValue(Func &&func){
        for(std::size_t k = 0;k < this->d3; k ++){
            for(std::size_t i = 0;i < this->d2; i ++){
                for(std::size_t j = 0;j < this->d1; j ++){
                    (*this)[k][i][j] = func((*this)[k][i][j]);
                }
            }
        }

        return *this;
    }

    template<class U, typename Func>
    Matrix3DBase<ValueType> &foreachFuncBinaryValue(const U &u, Func &&func){
        for(std::size_t k = 0;k < this->d3; k ++){
            for(std::size_t i = 0;i < this->d2; i ++){
                for(std::size_t j = 0;j < this->d1; j ++){
                    (*this)[k][i][j] = func((*this)[k][i][j], u);
                }
            }
        }
        
        return *this;
    }

    template<class U, typename Func>
    U foreachFuncTotal(Func &&func){
        U u{};
        for(std::size_t k = 0;k < this->d3; k ++){
            for(std::size_t i = 0;i < this->d2; i ++){
                for(std::size_t j = 0;j < this->d1; j ++){
                    u = func(u, (*this)[k][i][j]);
                }
            }
        }

        return u;
    }

public:
    template<class U>
    Matrix3DBase<ValueType>& operator+=(const Matrix3DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    Matrix3DBase<ValueType>& operator+=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    Matrix3DBase<ValueType>& operator-=(const Matrix3DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    Matrix3DBase<ValueType>& operator-=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    Matrix3DBase<ValueType>& operator*=(const Matrix3DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    Matrix3DBase<ValueType>& operator*=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    Matrix3DBase<ValueType>& operator/=(const Matrix3DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 / v2; });
    }

    template<class U>
    Matrix3DBase<ValueType>& operator/=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 / v2; });
    }

public:
    template<class U>
    U sum(){
        double acc = 0;
        for(std::size_t k = 0; k < this->d3; k++){
            for(std::size_t i = 0; i < this->d2; i++){
                for(std::size_t j = 0; j < this->d1; j++){
                    acc += static_cast<double>((*this)[k][i][j]);
                }
            }
        }
        return static_cast<U>(acc);
    }

    Matrix3DBase<ValueType>& eye(const ValueType v = 1 + ValueType{}){
        this->fill(ValueType{});
        auto size = std::min(this->d1, this->d2);
        for(std::size_t j = 0;j < this->d3; j++){
            for(std::size_t i = 0; i < size ;i ++){
                (*this)[j][i][i] = v;
            }
        }

        return *this;
    }

    Matrix3DBase<ValueType>& fill(const ValueType v = 1 + ValueType{}){
        return this->foreachFuncSingleValue([&v](const ValueType&){ return v; });
    }
};


template<class T, class U>
bool operator==(const Matrix3DBase<U> &m1, const Matrix3DBase<T> &m2){
    if(m2.d1 != m1.d1 || m2.d2 != m1.d2 || m1.d3 != m2.d3){
        return false;
    }

    for(std::size_t k = 0;k < m1.d3; k ++){
        for(std::size_t i = 0;i < m1.d2;i ++){
            for(std::size_t j = 0;j < m1.d1;j ++){
                if(m1[k][i][j] != m2[k][i][j]){
                    return false;
                }
            }
        }
    }
    
    return true;
}

template<class T, class U>
auto operator+(const Matrix3DBase<U> &m1, const Matrix3DBase<T> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1 && m1.d2 == m2.d2 && m1.d3 == m2.d3);
    Matrix3DBase<std::common_type_t<T, U>> ret(m1);
    return ret += m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix3DBase<T>>>>
auto operator+(const Matrix3DBase<T> &m1, const U &val){
    Matrix3DBase<std::common_type_t<T, U>> ret(m1);
    return ret += val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix3DBase<T>>>>
auto operator+(const U &val, const Matrix3DBase<T> &m1){
    return m1 + val;
}

template<class T, class U>
auto operator-(const Matrix3DBase<T> m1, const Matrix3DBase<U> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1 && m1.d2 == m2.d2 && m1.d3 == m2.d3);
    Matrix3DBase<std::common_type_t<T, U>> ret(m1);
    return ret -= m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix3DBase<T>>>>
auto operator-(const Matrix3DBase<T> m1, const U &val){
    Matrix3DBase<std::common_type_t<T, U>> ret(m1);
    return ret -= val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix3DBase<T>>>>
auto operator-(const U &val, const Matrix3DBase<T> m1){
    return val + ( -1 * m1);
}

template<class T, class U>
auto operator*(const Matrix3DBase<T> m1, const Matrix3DBase<U> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1 && m1.d2 == m2.d2 && m1.d3 == m2.d3);
    Matrix3DBase<std::common_type_t<T, U>> ret(m1);
    return ret *= m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix3DBase<T>>>>
auto operator*(const Matrix3DBase<T> m1, const U &val){
    Matrix3DBase<std::common_type_t<T, U>> ret(m1);
    return ret *= val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix3DBase<T>>>>
auto operator*(const U &val, const Matrix3DBase<T> m1){
    return m1 * val;
}

template<class T, class U>
auto operator/(const Matrix3DBase<T> &m1, const Matrix3DBase<U> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1 && m1.d2 == m2.d2 && m1.d3 == m2.d3);
    Matrix3DBase<std::common_type_t<T, U>> ret(m1);
    return ret /= m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix3DBase<T>>>>
auto operator/(const Matrix3DBase<T> &m1,const U &val){
    Matrix3DBase<std::common_type_t<T, U>> ret(m1);
    return ret /= val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix3DBase<T>>>>
auto operator/(const U &val, const Matrix3DBase<T> &m1){
    using ReturnType = std::common_type_t<T, U>;
    Matrix3DBase<ReturnType> ret(m1);
    ret.fill(ReturnType{} + 1);
    ret *= val;
    return ret / m1;
}
