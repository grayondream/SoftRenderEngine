#pragma once
#include "StaticMatrixIndex1.hpp"

template<class T, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class StaticMatrixIndex2 : public StaticMatrixIndexBase<T>{ 
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;

public:
    StaticMatrixIndex1<ValueType, d1> operator[](const StaticMatrixSizeType idx){
        return StaticMatrixIndex1<ValueType, d1>(this->m_pdata + idx * d1);
    }
};

template<class T, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class ConstStaticMatrixIndex2 : public ConstStaticMatrixIndexBase<T>{ 
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;

public:
    ConstStaticMatrixIndex1<ValueType, d1> operator[](const StaticMatrixSizeType idx) const{
        return ConstStaticMatrixIndex1<ValueType, d1>(this->m_pdata + idx * d1);
    }
};