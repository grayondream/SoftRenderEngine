#pragma once
#include "StaticMatrixIndex2.hpp"

template<class T, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class StaticMatrixIndex3 : public StaticMatrixIndexBase<T>{ 
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;

public:
    StaticMatrixIndex2<ValueType, d2, d1> operator[](const StaticMatrixSizeType idx){
        return { this->m_pdata + idx * d1 * d2};
    }
};

template<class T, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class ConstStaticMatrixIndex3 : public ConstStaticMatrixIndexBase<T>{ 
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;

public:
    ConstStaticMatrixIndex2<ValueType, d2, d1> operator[](const StaticMatrixSizeType idx) const {
        return { this->m_pdata + idx * d1 * d2};
    }
};