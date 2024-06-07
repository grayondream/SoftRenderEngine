#pragma once
#include "DynamicMatrix1D.hpp"
#include "DynamicMatrixBase.hpp"
#include "DynamicMatrix2DIndex.hpp"
#include "StaticMatrix2D.hpp"
#include "Vector2DBase.hpp"

template<class T>
class Matrix2DBase : public MatrixBase<T>, public MatrixIndex2<T>{
public:
    using ValueType = DynamicMatrixTraits<T>::ValueType;
    using ConstValueType = DynamicMatrixTraits<T>::ConstValueType;
    using Pointer = DynamicMatrixTraits<T>::Pointer;
    using ConstPointer = DynamicMatrixTraits<T>::ConstPointer;
    using Reference = DynamicMatrixTraits<T>::Reference;
    using ConstReference = DynamicMatrixTraits<T>::ConstReference;
    using MatrixDataType = DynamicMatrixTraits<T>::MatrixDataType;

public:
    Matrix2DBase(ConstMatrixSizeType d2, ConstMatrixSizeType d1) 
        : MatrixBase<ValueType>(d1 * d2), MatrixIndex2<ValueType>(this->getRawBuffer(), d2, d1){} 

    Matrix2DBase(const std::initializer_list<ValueType> &ls, ConstMatrixSizeType d2, ConstMatrixSizeType d1) 
        : MatrixBase<ValueType>(ls), MatrixIndex2<ValueType>(this->getRawBuffer(), d2, d1){
        assert(this->d2 * this->d1 == ls.size()); 
    } 

    Matrix2DBase(ConstPointer data, ConstMatrixSizeType d2, ConstMatrixSizeType d1)
        : MatrixBase<ValueType>(data, d2 * d1), MatrixIndex2<ValueType>(this->getRawBuffer(), d2, d1){}

    Matrix2DBase(const MatrixDataType &vec, ConstMatrixSizeType d2, ConstMatrixSizeType d1)
        : MatrixBase<ValueType>(vec), MatrixIndex2<ValueType>(this->getRawBuffer(), d2, d1){}

    Matrix2DBase(const MatrixIndex2<ValueType> &index)
        : MatrixBase<ValueType>(index.m_pstart, index.d1 * index.d2), MatrixIndex2<ValueType>(index){}
    
    Matrix2DBase(const std::initializer_list<std::initializer_list<ValueType>> &ls)
        : MatrixBase<ValueType>(ls.size() * ls.begin()->size()), MatrixIndex2<ValueType>(this->getRawBuffer(), ls.size(), ls.begin()->size()){
        Pointer p = this->getRawBuffer();
        for(auto &&l : ls){
            std::copy_n(std::begin(l), this->d1, p);
            p += this->d1;
        }
    }

    Matrix2DBase(const Matrix2DBase &mat)
        :   MatrixBase<ValueType>(mat.m_data), 
            MatrixIndex2<ValueType>(this->getRawBuffer(), mat.d2, mat.d1){}

    Matrix2DBase& operator=(const Matrix2DBase &mat){
        if(this == &mat) return *this;
        this->m_data = mat.m_data;
        this->m_pstart = mat.m_pstart;
        this->d1 = mat.d1;
        this->d2 = mat.d2;
        return *this;
    }

public:
    template<class U, typename Func>
    Matrix2DBase<ValueType>& foreachFuncBetweenMatrix(const Matrix2DBase<U> &mat, Func &&func){
        assert(mat.d1 > 0 && mat.d1 == this->d1 && mat.d2 == this->d2);
        for(auto i = 0;i < this->d2;i ++){
            for(auto j = 0;j < this->d1;j ++){
                (*this)[i][j] = func((*this)[i][j], mat[i][j]);
            }
        }
        
        return *this;
    }

    template<typename Func>
    Matrix2DBase<ValueType> &foreachFuncSingleValue(Func &&func){
        for(auto i = 0;i < this->d2;i ++){
            for(auto j = 0;j < this->d1;j ++){
                (*this)[i][j] = func((*this)[i][j]);
            }
        }

        return *this;
    }

    template<class U, typename Func>
    Matrix2DBase<ValueType> &foreachFuncBinaryValue(const U &u, Func &&func){
        for(auto i = 0;i < this->d2;i ++){
            for(auto j = 0;j < this->d1;j ++){
                (*this)[i][j] = func((*this)[i][j], u);
            }
        }
        
        return *this;
    }

    template<class U, typename Func>
    U foreachFuncTotal(Func &&func){
        U u{};
        for(auto i = 0;i < this->d2;i ++){
            for(auto j = 0;j < this->d1;j ++){
                u = func(u, (*this)[i][j]);
            }
        }

        return u;
    }

    Matrix2DBase<ValueType>& swapRows(const ConstMatrixSizeType rst, const ConstMatrixSizeType snd){
        Matrix1DBase<ValueType> r1 = (*this)[rst];
        (*this)[rst] = (*this)[snd];
        (*this)[snd] = r1;
        return *this;
    }
public:
    template<class U>
    Matrix2DBase<ValueType>& operator+=(const Matrix2DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    Matrix2DBase<ValueType>& operator+=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    Matrix2DBase<ValueType>& operator-=(const Matrix2DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    Matrix2DBase<ValueType>& operator-=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    Matrix2DBase<ValueType>& operator*=(const Matrix2DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    Matrix2DBase<ValueType>& operator*=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    Matrix2DBase<ValueType>& operator/=(const Matrix2DBase<U> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 / v2; });
    }

    template<class U>
    Matrix2DBase<ValueType>& operator/=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 / v2; });
    }

    template<class U>
    Matrix2DBase<ValueType> mul(const Matrix2DBase<U> &mat){
        assert(this->d1 == mat.d2);
        //d12 x d11 X d22 x d21
        //d11 ==d22
        //return d12 x d21
        Matrix2DBase<ValueType> ret(this->d2, mat.d1);
        for(auto i = 0;i < this->d2;i ++){
            for(auto j = 0;j < mat.d1;j++){
                ValueType sum{};
                for(auto k = 0;k < mat.d2;k ++){
                    sum += (*this)[i][k] * mat[k][j];
                }
                ret[i][j] = sum;
            }
        }

        return ret;
    }

    template<class U, StaticMatrixSizeType di2, StaticMatrixSizeType di1>
    Matrix2DBase<ValueType> mul(const StaticMatrix2DBase<U, di2, di1> &mat){
        assert(this->d1 == mat.d2);
        //d12 x d11 X d22 x d21
        //d11 ==d22
        //return d12 x d21
        Matrix2DBase<ValueType> ret(this->d2, mat.d1);
        for(auto i = 0;i < this->d2;i ++){
            for(auto j = 0;j < mat.d1;j++){
                ValueType sum{};
                for(auto k = 0;k < mat.d2;k ++){
                    sum += (*this)[i][k] * mat[k][j];
                }
                ret[i][j] = sum;
            }
        }

        return ret;
    }

public:
    template<class U>
    U sum(){
        return this->foreachFuncTotal<U>([](const ValueType v1, const ValueType v2){ return v1 + v2; });
    }

    Matrix2DBase<ValueType>& eye(const ValueType v = 1 + ValueType{}){
        auto size = std::min(this->d1, this->d2);
        for(auto i = 0; i < size ;i ++){
            (*this)[i][i] = v;
        }

        return *this;
    }

    Matrix2DBase<ValueType>& fill(const ValueType v = 1 + ValueType{}){
        return this->foreachFuncSingleValue([&v](const ValueType&){ return v; });
    }

    auto runk() const{
        //高斯消元法计算矩阵的秩，代码有些问题后面再修复
        //TODO:
        Matrix2DBase<ValueType> mat = *this;
        MatrixSizeType rank = 0;
        auto m = mat.d2, n = mat.d1;
        for (int row = 0; row < m; ++row) {
            // If the leading element in the current row is zero, try to swap with a row below it
            if (mat[row][rank] == 0) {
                bool found = false;
                for (int i = row + 1; i < m; ++i) {
                    if (mat[i][rank] != 0) {
                        mat.swapRows(row, i);
                        found = true;
                        break;
                    }
                }
                // If no non-zero element is found, move to the next column
                if (!found) {
                    if (++rank >= n) {
                        return rank;
                    }
                    --row;
                    continue;
                }
            }

            // Normalize the leading element to 1
            double leading = mat[row][rank];
            for (int col = 0; col < n; ++col) {
                mat[row][col] /= leading;
            }

            // Make all rows below this one 0 in the current column
            for (int i = row + 1; i < m; ++i) {
                double factor = mat[i][rank];
                for (int col = 0; col < n; ++col) {
                    mat[i][col] -= factor * mat[row][col];
                }
            }

            ++rank;
        }

        return rank;
    }

    Matrix2DBase<ValueType> transpose() const{
        Matrix2DBase<ValueType> ret(this->d1, this->d2);
        for(auto i = 0;i < ret.d2;i ++){
            for(auto j = 0;j < ret.d1;j ++){
                ret[i][j] = (*this)[j][i];
            }
        }

        return ret;
    }
}; 

template<class T, class U>
bool operator==(const Matrix2DBase<T> &m1, const Matrix2DBase<U> &m2){
    if(m2.d1 != m1.d1 || m2.d2 != m1.d2){
        return false;
    }

    for(auto i = 0;i < m1.d2;i ++){
        for(auto j = 0;j < m1.d1;j ++){
            if(m1[i][j] != m2[i][j]){
                return false;
            }
        }
    }

    return true;
}

template<class T, class U>
auto operator+(const Matrix2DBase<U> &m1, const Matrix2DBase<T> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1);
    Matrix2DBase<std::common_type_t<T, U>> ret(m1);
    return ret += m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix2DBase<T>>>>
auto operator+(const Matrix2DBase<T> &m1, const U &val){
    Matrix2DBase<std::common_type_t<T, U>> ret(m1);
    return ret += val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix2DBase<T>>>>
auto operator+(const U &val, const Matrix2DBase<T> &m1){
    return m1 + val;
}

template<class T, class U>
auto operator-(const Matrix2DBase<T> m1, const Matrix2DBase<U> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1);
    Matrix2DBase<std::common_type_t<T, U>> ret(m1);
    return ret -= m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix2DBase<T>>>>
auto operator-(const Matrix2DBase<T> m1, const U &val){
    Matrix2DBase<std::common_type_t<T, U>> ret(m1);
    return ret -= val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix2DBase<T>>>>
auto operator-(const U &val, const Matrix2DBase<T> m1){
    return val + ( -1 * m1);
}

template<class T, class U>
auto operator*(const Matrix2DBase<T> m1, const Matrix2DBase<U> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1);
    Matrix2DBase<std::common_type_t<T, U>> ret(m1);
    return ret *= m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix2DBase<T>>>>
auto operator*(const Matrix2DBase<T> m1, const U &val){
    Matrix2DBase<std::common_type_t<T, U>> ret(m1);
    return ret *= val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix2DBase<T>>>>
auto operator*(const U &val, const Matrix2DBase<T> m1){
    return m1 * val;
}

template<class T, class U>
auto operator/(const Matrix2DBase<T> &m1, const Matrix2DBase<U> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1);
    Matrix2DBase<std::common_type_t<T, U>> ret(m1);
    return ret /= m2;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix2DBase<T>>>>
auto operator/(const Matrix2DBase<T> &m1,const U &val){
    Matrix2DBase<std::common_type_t<T, U>> ret(m1);
    return ret /= val;
}

template<class T, class U, typename = std::enable_if_t<!std::is_same_v<U, Matrix2DBase<T>>>>
auto operator/(const U &val, const Matrix2DBase<T> &m1){
    using ReturnType = std::common_type_t<T, U>;
    Matrix2DBase<ReturnType> ret(m1);
    ret.fill(ReturnType{} + 1);
    ret *= val;
    return ret / m1;
}