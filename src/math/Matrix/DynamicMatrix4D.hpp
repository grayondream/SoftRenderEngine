#pragma once

#pragma once
#include "DynamicMatrixBase.hpp"
#include "DynamicMatrix4DIndex.hpp"
#include "Vector4DBase.hpp"

template<class T>
class Matrix4DBase : public MatrixBase<T>, public MatrixIndex4<T>{
public:
    using ValueType = DynamicMatrixTraits<T>::ValueType;
    using ConstValueType = DynamicMatrixTraits<T>::ConstValueType;
    using Pointer = DynamicMatrixTraits<T>::Pointer;
    using ConstPointer = DynamicMatrixTraits<T>::ConstPointer;
    using Reference = DynamicMatrixTraits<T>::Reference;
    using ConstReference = DynamicMatrixTraits<T>::ConstReference;
    using MatrixDataType = DynamicMatrixTraits<T>::MatrixDataType;

public:
    Matrix4DBase(ConstMatrixSizeType d4, ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1) 
        : MatrixBase<ValueType>(d1 * d2 * d3 * d4), MatrixIndex4<ValueType>(this->getRawBuffer(), d4, d3, d2, d1){ }

    Matrix4DBase(const std::initializer_list<ValueType> &ls, ConstMatrixSizeType d4, ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1)
        : MatrixBase<ValueType>(d4 * d3 * d2 * d1), MatrixIndex4<ValueType>(this->getRawBuffer(), d4, d3, d2, d1){
        assert(ls.size() == d4 * d3 * d2 * d1);
    }
    Matrix4DBase(ConstPointer data, ConstMatrixSizeType d4, ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1) 
        : MatrixBase<ValueType>(data, d1 * d2 * d3 * d4), MatrixIndex4<ValueType>(this->getRawBuffer(), d4, d3, d2, d1){ }

    Matrix4DBase(const MatrixDataType &vec, ConstMatrixSizeType d4, ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1) 
        : MatrixBase<ValueType>(vec), MatrixIndex4<ValueType>(this->getRawBuffer(), d4, d3, d2, d1){ }

    Matrix4DBase(const MatrixIndex4<ValueType> &index)
        : MatrixBase<ValueType>(index.m_pstart, index.d1 * index.d2 * index.d3 * index.d4), MatrixIndex4<ValueType>(index){}

    Matrix4DBase(const std::initializer_list<std::initializer_list<std::initializer_list<std::initializer_list<ValueType>>>> &ls)
        :   MatrixBase<ValueType>(ls.size() * ls.begin()->size() * ls.begin()->begin()->size() * ls.begin()->begin()->begin()->size()),
            MatrixIndex4<ValueType>(this->getRawBuffer(), ls.size(), ls.begin()->size(), ls.begin()->begin()->size(), ls.begin()->begin()->begin()->size()){
        assert(ls.size() > 0);
        assert(ls.begin()->size() == this->d3 && ls.begin()->begin()->size() == this->d2 && ls.begin()->begin()->begin()->size() == this->d1);
        Pointer p = this->getRawBuffer();
        for(auto && c : ls){
            assert(c.size() == this->d3);
            for(auto &&r : c){
                assert(r.size() == this->d2);
                for(auto && l : r){
                    assert(l.size() == this->d1);
                    std::copy_n(std::begin(l), this->d1, p);
                    p += this->d1;
                }
            }
        }
    }

    Matrix4DBase(const Matrix4DBase &mat)
    :   MatrixBase<ValueType>(mat.m_data), 
        MatrixIndex4<ValueType>(this->getRawBuffer(), mat.d4, mat.d3, mat.d2, mat.d1){}

    Matrix4DBase& operator=(const Matrix4DBase &mat){
        if(this == &mat) return *this;
        this->m_data = mat.m_data;
        this->m_pstart = this->getRawBuffer();
        this->d1 = mat.d1;
        this->d2 = mat.d2;
        this->d3 = mat.d3;
        this->d4 = mat.d4;
        return *this;
    }

public:
    template<class U, typename Func>
    Matrix4DBase<ValueType>& foreachFuncBetweenMatrix(const Matrix4DBase<U> &mat, Func &&func){
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
    Matrix4DBase<ValueType> &foreachFuncSingleValue(Func &&func){
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
    Matrix4DBase<ValueType> &foreachFuncBinaryValue(const U &u, Func &&func){
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
    Matrix4DBase<ValueType>& operator+=(const Matrix4DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    Matrix4DBase<ValueType>& operator+=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    Matrix4DBase<ValueType>& operator-=(const Matrix4DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    Matrix4DBase<ValueType>& operator-=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    Matrix4DBase<ValueType>& operator*=(const Matrix4DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    Matrix4DBase<ValueType>& operator*=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    Matrix4DBase<ValueType>& operator/=(const Matrix4DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 / v2; });
    }

    template<class U>
    Matrix4DBase<ValueType>& operator/=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 / v2; });
    }

public:
    template<class U>
    U sum(){
        double acc = 0;
        for(auto c = 0; c < this->d4; c++){
            for(auto k = 0; k < this->d3; k++){
                for(auto i = 0; i < this->d2; i++){
                    for(auto j = 0; j < this->d1; j++){
                        acc += static_cast<double>((*this)[c][k][i][j]);
                    }
                }
            }
        }
        return static_cast<U>(acc);
    }

    Matrix4DBase<ValueType>& eye(const ValueType v = 1 + ValueType{}){
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

    Matrix4DBase<ValueType>& fill(const ValueType v = 1 + ValueType{}){
        return this->foreachFuncSingleValue([&v](const ValueType&){ return v; });
    }

    template<class U>
    Matrix4DBase<ValueType> mul(const Matrix4DBase<U> &mat) const{
        assert(this->d1 == mat.d2);
        Matrix4DBase<ValueType> ret(this->d4, this->d3, this->d2, mat.d1);
        for(auto c = 0; c < this->d4; c++){
            for(auto k = 0; k < this->d3; k++){
                for(auto i = 0; i < this->d2; i++){
                    for(auto j = 0; j < mat.d1; j++){
                        ValueType sum{};
                        for(auto s = 0; s < this->d1; s++){
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
        return Vector4DBase<ValueType>({
            (*this)[0][0][0][0]*vec.x + (*this)[0][0][0][1]*vec.y + (*this)[0][0][0][2]*vec.z + (*this)[0][0][0][3]*vec.w,
            (*this)[0][0][1][0]*vec.x + (*this)[0][0][1][1]*vec.y + (*this)[0][0][1][2]*vec.z + (*this)[0][0][1][3]*vec.w,
            (*this)[0][0][2][0]*vec.x + (*this)[0][0][2][1]*vec.y + (*this)[0][0][2][2]*vec.z + (*this)[0][0][2][3]*vec.w,
            (*this)[0][0][3][0]*vec.x + (*this)[0][0][3][1]*vec.y + (*this)[0][0][3][2]*vec.z + (*this)[0][0][3][3]*vec.w
        });
    }
};

template<class T, class U>
bool operator==(const Matrix4DBase<U> &m1, const Matrix4DBase<T> &m2){
    if(m2.d1 != m1.d1 || m2.d2 != m1.d2 || m1.d3 != m2.d3 || m2.d4 != m1.d4){
        return false;
    }

    for(auto c = 0; c < m1.d4; c ++){
        for(auto k = 0;k < m1.d3; k ++){
            for(auto i = 0;i < m1.d2;i ++){
                for(auto j = 0;j < m1.d1;j ++){
                    if(m1[c][k][i][j] != m2[c][k][i][j]){
                        return false;
                    }
                }
            }
        }
    }    

    return true;
}

template<class T, class U>
auto operator+(const Matrix4DBase<U> &m1, const Matrix4DBase<T> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1 && m1.d2 == m2.d2 && m1.d3 == m2.d3 && m1.d4 == m2.d4);
    Matrix4DBase<std::common_type_t<T, U>> ret(m1);
    return ret += m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix4DBase<T>>>>
auto operator+(const Matrix4DBase<T> &m1, const U &val){
    Matrix4DBase<std::common_type_t<T, U>> ret(m1);
    return ret += val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix4DBase<T>>>>
auto operator+(const U &val, const Matrix4DBase<T> &m1){
    return m1 + val;
}

template<class T, class U>
auto operator-(const Matrix4DBase<T> m1, const Matrix4DBase<U> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1 && m1.d2 == m2.d2 && m1.d3 == m2.d3 && m1.d4 == m2.d4);
    Matrix4DBase<std::common_type_t<T, U>> ret(m1);
    return ret -= m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix4DBase<T>>>>
auto operator-(const Matrix4DBase<T> m1, const U &val){
    Matrix4DBase<std::common_type_t<T, U>> ret(m1);
    return ret -= val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix4DBase<T>>>>
auto operator-(const U &val, const Matrix4DBase<T> m1){
    return val + ( -1 * m1);
}

template<class T, class U>
auto operator*(const Matrix4DBase<T> m1, const Matrix4DBase<U> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1 && m1.d2 == m2.d2 && m1.d3 == m2.d3 && m1.d4 == m2.d4);
    Matrix4DBase<std::common_type_t<T, U>> ret(m1);
    return ret *= m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix4DBase<T>>>>
auto operator*(const Matrix4DBase<T> m1, const U &val){
    Matrix4DBase<std::common_type_t<T, U>> ret(m1);
    return ret *= val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix4DBase<T>>>>
auto operator*(const U &val, const Matrix4DBase<T> m1){
    return m1 * val;
}

template<class T, class U>
auto operator/(const Matrix4DBase<T> &m1, const Matrix4DBase<U> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1 && m1.d2 == m2.d2 && m1.d3 == m2.d3 && m1.d4 == m2.d4);
    Matrix4DBase<std::common_type_t<T, U>> ret(m1);
    return ret /= m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix4DBase<T>>>>
auto operator/(const Matrix4DBase<T> &m1,const U &val){
    Matrix4DBase<std::common_type_t<T, U>> ret(m1);
    return ret /= val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix4DBase<T>>>>
auto operator/(const U &val, const Matrix4DBase<T> &m1){
    using ReturnType = std::common_type_t<T, U>;
    Matrix4DBase<ReturnType> ret(m1);
    ret.fill(ReturnType{} + 1);
    ret *= val;
    return ret / m1;
}