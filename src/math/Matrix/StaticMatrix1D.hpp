
#pragma once
#include <algorithm>
#include <initializer_list>
#include <vector>
#include <cassert>
#include "StaticMatrixIndex1.hpp"


template<class T, StaticMatrixSizeType di1>
class StaticMatrix1DBase{
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;
    using ConstValueType = StaticMatrixTraits<T>::ConstValueType;
    using Pointer = StaticMatrixTraits<T>::Pointer;
    using ConstPointer = StaticMatrixTraits<T>::ConstPointer;
    using Reference = StaticMatrixTraits<T>::Reference;
    using ConstReference = StaticMatrixTraits<T>::ConstReference;
 
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
private:
    ValueType m_pdata[d1]{};
};