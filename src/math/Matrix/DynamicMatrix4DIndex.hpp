#pragma once
#include "DynamicMatrixBase.hpp"
#include "DynamicMatrix3DIndex.hpp"

template<class T>
class MatrixIndex4 : public MatrixIndexBase<T>, public MatrixIndex4Size{
public:
    using ValueType = DynamicMatrixTraits<T>::ValueType;
    using ConstValueType = DynamicMatrixTraits<T>::ConstValueType;
    using Pointer = DynamicMatrixTraits<T>::Pointer;
    using ConstPointer = DynamicMatrixTraits<T>::ConstPointer;
    using Reference = DynamicMatrixTraits<T>::Reference;
    using ConstReference = DynamicMatrixTraits<T>::ConstReference;

public:
    MatrixIndex4(Pointer data, ConstMatrixSizeType d4, ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1) 
            : MatrixIndexBase<ValueType>(data), MatrixIndex4Size(d4, d3, d2, d1){ }

    MatrixIndex4(ConstPointer data, ConstMatrixSizeType d4, ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1) 
            : MatrixIndexBase<ValueType>(data), MatrixIndex4Size(d4, d3, d2, d1){ }

    MatrixIndex3<ValueType> operator[](ConstMatrixSizeType idx) const {
        return MatrixIndex3<ValueType>(this->m_pstart + idx * this->d1 * this->d2 * this->d3, this->d3, this->d2, this->d1);
    }

    MatrixIndex3<ValueType> operator[](ConstMatrixSizeType idx){
        return MatrixIndex3<ValueType>(this->m_pstart + idx * this->d1 * this->d2 * this->d3, this->d3, this->d2, this->d1);
    }
};