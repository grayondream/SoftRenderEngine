#pragma once
#include "StaticMatrixIndex3.hpp"

template<class T, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class StaticMatrixIndex4 : public StaticMatrixIndexBase<T>{ 
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;

public:
    StaticMatrixIndex3<ValueType, d3, d2, d1> operator[](const StaticMatrixSizeType idx){
        return { this->m_pdata + idx * d3 * d2 * d1};
    }
};

template<class T, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class ConstStaticMatrixIndex4 : public ConstStaticMatrixIndexBase<T>{ 
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;

public:
    ConstStaticMatrixIndex3<ValueType, d3, d2, d1> operator[](const StaticMatrixSizeType idx) const{
        return { this->m_pdata + idx * d3 * d2 * d1};
    }
};