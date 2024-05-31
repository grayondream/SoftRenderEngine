#pragma once
#include <algorithm>
#include <initializer_list>
#include <vector>
#include <cassert>
#include "StaticMatrixIndex2.hpp"

template<class T, StaticMatrixSizeType di2, StaticMatrixSizeType di1>
class StaticMatrix2DBase{
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;
    using ConstValueType = StaticMatrixTraits<T>::ConstValueType;
    using Pointer = StaticMatrixTraits<T>::Pointer;
    using ConstPointer = StaticMatrixTraits<T>::ConstPointer;
    using Reference = StaticMatrixTraits<T>::Reference;
    using ConstReference = StaticMatrixTraits<T>::ConstReference;
 
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
private:
    ValueType m_pdata[d2][d1]{};
};