#pragma once
#include "StaticMatrixBase.hpp"

template<class T, StaticMatrixSizeType di1>
class StaticMatrixIndex1 : public StaticMatrixIndexBase<T>{
public:
    using ValueType = typename StaticMatrixTraits<T>::ValueType;

public:
    ValueType& operator[](const StaticMatrixSizeType idx){
        return this->m_pdata[idx];
    }
};

template<class T, StaticMatrixSizeType di1>
class ConstStaticMatrixIndex1 : public ConstStaticMatrixIndexBase<T>{
public:
    using ValueType = typename StaticMatrixTraits<T>::ValueType;

public:
    ValueType operator[](const StaticMatrixSizeType idx) const {
        return this->m_pdata[idx];
    }
};