#pragma once
#include "DynamicMatrixBase.hpp"
#include "DynamicMatrix2DIndex.hpp"
#include <cassert>

template<class T>
class MatrixIndex3 : public MatrixIndexBase<T>, public MatrixIndex3Size{
public:
    using ValueType = DynamicMatrixTraits<T>::ValueType;
    using ConstValueType = DynamicMatrixTraits<T>::ConstValueType;
    using Pointer = DynamicMatrixTraits<T>::Pointer;
    using ConstPointer = DynamicMatrixTraits<T>::ConstPointer;
    using Reference = DynamicMatrixTraits<T>::Reference;
    using ConstReference = DynamicMatrixTraits<T>::ConstReference;

public:
    MatrixIndex3(Pointer data, ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1) 
        : MatrixIndexBase<ValueType>(data), MatrixIndex3Size(d3, d2, d1){}

    MatrixIndex3(ConstPointer data, ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1) 
        : MatrixIndexBase<ValueType>(data), MatrixIndex3Size(d3, d2, d1){}

    MatrixIndex2<ValueType> operator[](ConstMatrixSizeType idx) const {
        assert(idx < this->d3);
        return MatrixIndex2<ValueType>(this->m_pstart + idx * this->d1 * this->d2, this->d2, this->d1);
    }

    MatrixIndex2<ValueType> operator[](ConstMatrixSizeType idx){
        assert(idx < this->d3);
        return const_cast<const MatrixIndex3<ValueType>*>(this)->operator[](idx);
    }
};