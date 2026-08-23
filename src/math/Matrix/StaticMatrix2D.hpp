#pragma once
#include <algorithm>
#include <initializer_list>
#include <vector>
#include <cassert>
#include "StaticMatrixIndex2.hpp"
#include "StaticMatrixBase.hpp"

template<class T, StaticMatrixSizeType di2, StaticMatrixSizeType di1>
class StaticMatrix2DBase{
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
    constexpr static const StaticMatrixSizeType Size = d2 * d1;
public:
    StaticMatrix2DBase() = default;
    StaticMatrix2DBase(const std::vector<ValueType> &vec){
        assert(vec.size() == size());
        std::copy_n(std::begin(vec), size(), getRawBuffer());
    }
    
    StaticMatrix2DBase(const std::initializer_list<ValueType> &ls){
        assert(ls.size() == size());
        std::copy_n(std::begin(ls), size(), getRawBuffer());
    }

    StaticMatrix2DBase(const std::initializer_list<std::initializer_list<ValueType>> &ls){
        assert(ls.size() == d2);
        assert(ls.begin()->size() == d1);
        Pointer p = getRawBuffer();
        for(auto &&l : ls){
            std::copy_n(std::begin(l), d1, p);
            p += d1;
        }
    }

    StaticMatrix2DBase(Pointer data){
        std::copy_n(data, size(), getRawBuffer());
    }
    
    StaticMatrix2DBase(const StaticMatrixIndex2<ValueType, d2, d1> &mat){
        std::copy_n(mat.m_pdata, size(), getRawBuffer());
    }

    StaticMatrix2DBase(const ConstStaticMatrixIndex2<ValueType, d2, d1> &mat){
        std::copy_n(mat.m_pdata, size(), getRawBuffer());
    }

    StaticMatrix2DBase(const StaticMatrix2DBase &mat){
        std::copy_n(mat.getRawBuffer(), mat.size(), getRawBuffer());
    }

    StaticMatrix2DBase& operator=(const StaticMatrix2DBase &mat){
        if(this == &mat){
            return *this;
        }

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

    ConstPointer getRawBuffer() const {
        return (ConstPointer)(m_pdata);
    }

    Pointer getRawBuffer(){
        return reinterpret_cast<Pointer>(m_pdata);
    }

    const ConstStaticMatrixIndex1<ValueType, d1> operator[](const StaticMatrixSizeType idx) const {
        return ConstStaticMatrixIndex1<ValueType, d1>(getRawBuffer() + idx * d1);
    }

    StaticMatrixIndex1<ValueType, d1> operator[](const StaticMatrixSizeType idx){
        return StaticMatrixIndex1<ValueType, d1>(getRawBuffer() + idx * d1);
    }

public:
    template<class U, StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename Func>
    StaticMatrix2DBase<ValueType, d2,d1>& foreachFuncBetweenMatrix(const StaticMatrix2DBase<U, d2, d1> &mat, Func &&func){
        assert(mat.d1 > 0 && mat.d1 == this->d1 && mat.d2 == this->d2);
        for(StaticMatrixSizeType i = 0;i < this->d2;i ++){
            for(StaticMatrixSizeType j = 0;j < this->d1;j ++){
                (*this)[i][j] = func((*this)[i][j], mat[i][j]);
            }
        }
        
        return *this;
    }

    template<typename Func>
    StaticMatrix2DBase<ValueType, d2,d1> &foreachFuncSingleValue(Func &&func){
        for(StaticMatrixSizeType i = 0;i < this->d2;i ++){
            for(StaticMatrixSizeType j = 0;j < this->d1;j ++){
                (*this)[i][j] = func((*this)[i][j]);
            }
        }

        return *this;
    }

    template<class U, typename Func>
    StaticMatrix2DBase<ValueType, d2,d1> &foreachFuncBinaryValue(const U &u, Func &&func){
        for(StaticMatrixSizeType i = 0;i < this->d2;i ++){
            for(StaticMatrixSizeType j = 0;j < this->d1;j ++){
                (*this)[i][j] = func((*this)[i][j], u);
            }
        }
        
        return *this;
    }

    template<class U, typename Func>
    U foreachFuncTotal(Func &&func){
        U u{};
        for(StaticMatrixSizeType i = 0;i < this->d2;i ++){
            for(StaticMatrixSizeType j = 0;j < this->d1;j ++){
                u = func(u, (*this)[i][j]);
            }
        }

        return u;
    }

public:
    template<class U>
    bool operator==(const StaticMatrix2DBase<U, d2, d1> &mat) const{
        if(mat.d1 != this->d1 || mat.d2 != this->d2){
            return false;
        }

        for(StaticMatrixSizeType i = 0;i < this->d2;i ++){
            for(StaticMatrixSizeType j = 0;j < this->d1;j ++){
                if((*this)[i][j] != mat[i][j]){
                    return false;
                }
            }
        }

        return true;
    }

    template<class U>
    StaticMatrix2DBase<ValueType, d2,d1>& operator+=(const StaticMatrix2DBase<U, d2, d1> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    StaticMatrix2DBase<ValueType, d2,d1>& operator+=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 + v2; });
    }

    template<class U>
    StaticMatrix2DBase<ValueType, d2,d1>& operator-=(const StaticMatrix2DBase<U, d2, d1> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    StaticMatrix2DBase<ValueType, d2,d1>& operator-=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 - v2; });
    }

    template<class U>
    StaticMatrix2DBase<ValueType, d2,d1>& operator*=(const StaticMatrix2DBase<U, d2, d1> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    StaticMatrix2DBase<ValueType, d2,d1>& operator*=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 * v2; });
    }

    template<class U>
    StaticMatrix2DBase<ValueType, d2,d1>& operator/=(const StaticMatrix2DBase<U, d2, d1> &mat){
        return this->foreachFuncBetweenMatrix(mat, [](const T &v1, const U &v2){ return v1 / v2; });
    }

    template<class U>
    StaticMatrix2DBase<ValueType, d2,d1>& operator/=(const U &val){
        return this->foreachFuncBinaryValue(val, [](const T &v1, const U &v2){ return v1 / v2; });
    }

public:
    template<class U>
    U sum(){
        double acc = 0;
        for(StaticMatrixSizeType i = 0; i < d2; i++){
            for(StaticMatrixSizeType j = 0; j < d1; j++){
                acc += static_cast<double>((*this)[i][j]);
            }
        }
        return static_cast<U>(acc);
    }

    StaticMatrix2DBase<ValueType, d2,d1>& eye(const ValueType v = 1 + ValueType{}){
        this->fill(ValueType{});
        auto size = std::min(this->d1, this->d2);
        for(StaticMatrixSizeType i = 0; i < size ;i ++){
            (*this)[i][i] = v;
        }

        return *this;
    }

    StaticMatrix2DBase<ValueType, d2,d1>& fill(const ValueType v = 1 + ValueType{}){
        return this->foreachFuncSingleValue([&v](const ValueType&){ return v; });
    }


private:
    ValueType m_pdata[d2][d1]{};
};

template<class T, class U , StaticMatrixSizeType d2, StaticMatrixSizeType d1>
auto operator+(const StaticMatrix2DBase<U, d2, d1> &m1, const StaticMatrix2DBase<T, d2, d1> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1 && m1.d2 == m2.d2);
    StaticMatrix2DBase<std::common_type_t<T, U>, d2, d1> ret(m1);
    return ret += m2;
}

template<class T, class U , StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix2DBase<T, d2, d1>>>>
auto operator+(const StaticMatrix2DBase<T, d2, d1> &m1, const U &val){
    StaticMatrix2DBase<std::common_type_t<T, U>, d2, d1> ret(m1);
    return ret += val;
}

template<class T, class U , StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix2DBase<T, d2, d1>>>>
auto operator+(const U &val, const StaticMatrix2DBase<T, d2, d1> &m1){
    return m1 + val;
}

template<class T, class U , StaticMatrixSizeType d2, StaticMatrixSizeType d1>
auto operator-(const StaticMatrix2DBase<T, d2, d1> m1, const StaticMatrix2DBase<U, d2, d1> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1 && m1.d2 == m2.d2);
    StaticMatrix2DBase<std::common_type_t<T, U>, d2, d1> ret(m1);
    return ret -= m2;
}

template<class T, class U , StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix2DBase<T, d2, d1>>>>
auto operator-(const StaticMatrix2DBase<T, d2, d1> m1, const U &val){
    StaticMatrix2DBase<std::common_type_t<T, U>, d2, d1> ret(m1);
    return ret -= val;
}

template<class T, class U , StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix2DBase<T, d2, d1>>>>
auto operator-(const U &val, const StaticMatrix2DBase<T, d2, d1> m1){
    return val + ( -1 * m1);
}

template<class T, class U , StaticMatrixSizeType d2, StaticMatrixSizeType d1>
auto operator*(const StaticMatrix2DBase<T, d2, d1> m1, const StaticMatrix2DBase<U, d2, d1> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1 && m1.d2 == m2.d2);
    StaticMatrix2DBase<std::common_type_t<T, U>, d2, d1> ret(m1);
    return ret *= m2;
}

template<class T, class U , StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix2DBase<T, d2, d1>>>>
auto operator*(const StaticMatrix2DBase<T, d2, d1> m1, const U &val){
    StaticMatrix2DBase<std::common_type_t<T, U>, d2, d1> ret(m1);
    return ret *= val;
}

template<class T, class U , StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix2DBase<T, d2, d1>>>>
auto operator*(const U &val, const StaticMatrix2DBase<T, d2, d1> m1){
    return m1 * val;
}

template<class T, class U , StaticMatrixSizeType d2, StaticMatrixSizeType d1>
auto operator/(const StaticMatrix2DBase<T, d2, d1> &m1, const StaticMatrix2DBase<U, d2, d1> &m2){
    assert(m1.d1 > 0 && m1.d1 == m2.d1 && m1.d2 == m2.d2);
    StaticMatrix2DBase<std::common_type_t<T, U>, d2, d1> ret(m1);
    return ret /= m2;
}

template<class T, class U , StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix2DBase<T, d2, d1>>>>
auto operator/(const StaticMatrix2DBase<T, d2, d1> &m1,const U &val){
    StaticMatrix2DBase<std::common_type_t<T, U>, d2, d1> ret(m1);
    return ret /= val;
}

template<class T, class U , StaticMatrixSizeType d2, StaticMatrixSizeType d1, typename = std::enable_if_t<!std::is_same_v<U, StaticMatrix2DBase<T, d2, d1>>>>
auto operator/(const U &val, const StaticMatrix2DBase<T, d2, d1> &m1){
    using ReturnType = std::common_type_t<T, U>;
    StaticMatrix2DBase<ReturnType, d2, d1> ret(m1);
    ret.fill(ReturnType{} + 1);
    ret *= val;
    return ret / m1;
}