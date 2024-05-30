#pragma once
#include "DynamicMatrixBase.hpp"

template<class T>
class MatrixIndex1 : public MatrixIndexBase<T>, public MatrixIndex1Size{
public:
    using ValueType = DynamicMatrixTraits<T>::ValueType;
    using ConstValueType = DynamicMatrixTraits<T>::ConstValueType;
    using Pointer = DynamicMatrixTraits<T>::Pointer;
    using ConstPointer = DynamicMatrixTraits<T>::ConstPointer;
    using Reference = DynamicMatrixTraits<T>::Reference;
    using ConstReference = DynamicMatrixTraits<T>::ConstReference;
    
public:
    MatrixIndex1(Pointer data, ConstMatrixSizeType d1) 
        : MatrixIndexBase<ValueType>(data), MatrixIndex1Size(d1){ }
    
    MatrixIndex1(ConstPointer data, ConstMatrixSizeType d1)
        : MatrixIndexBase<ValueType>(data), MatrixIndex1Size(d1){}

    Reference operator[](ConstMatrixSizeType idx){
        return *(this->m_pstart + idx);
    }
    
    ValueType operator[](ConstMatrixSizeType idx) const{
        return *(this->m_pstart + idx);
    }
};