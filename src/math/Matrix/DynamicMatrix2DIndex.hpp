#pragma once
#include "DynamicMatrixBase.hpp"
#include "DynamicMatrix1DIndex.hpp"

//d1 -> width
template<class T>
class MatrixIndex2 : public MatrixIndexBase<T>, public MatrixIndex2Size{
public:
    using ValueType = DynamicMatrixTraits<T>::ValueType;
    using ConstValueType = DynamicMatrixTraits<T>::ConstValueType;
    using Pointer = DynamicMatrixTraits<T>::Pointer;
    using ConstPointer = DynamicMatrixTraits<T>::ConstPointer;
    using Reference = DynamicMatrixTraits<T>::Reference;
    using ConstReference = DynamicMatrixTraits<T>::ConstReference;

public:
    MatrixIndex2(Pointer data, ConstMatrixSizeType d2, ConstMatrixSizeType d1) 
        : MatrixIndexBase<ValueType>(data), MatrixIndex2Size(d2, d1){ }
    
    MatrixIndex2(ConstPointer data, ConstMatrixSizeType d2, ConstMatrixSizeType d1) 
        : MatrixIndexBase<ValueType>(data), MatrixIndex2Size(d2, d1){ }

    MatrixIndex1<ValueType> operator[](ConstMatrixSizeType idx) const{
        return MatrixIndex1<ValueType>(this->m_pstart + idx * this->d1, this->d1);
    }

    MatrixIndex1<ValueType> operator[](ConstMatrixSizeType idx){
        return const_cast<const MatrixIndex2<ValueType>*>(this)->operator[](idx);
    }
};