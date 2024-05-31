#pragma once
#include "DynamicMatrixBase.hpp"
#include "DynamicMatrix1DIndex.hpp"

template<class T>
class Matrix1DBase : public MatrixBase<T>, public MatrixIndex1<T>{
public:
    using ValueType = DynamicMatrixTraits<T>::ValueType;
    using ConstValueType = DynamicMatrixTraits<T>::ConstValueType;
    using Pointer = DynamicMatrixTraits<T>::Pointer;
    using ConstPointer = DynamicMatrixTraits<T>::ConstPointer;
    using Reference = DynamicMatrixTraits<T>::Reference;
    using ConstReference = DynamicMatrixTraits<T>::ConstReference;
    using MatrixDataType = DynamicMatrixTraits<T>::MatrixDataType;

public:
    Matrix1DBase(ConstMatrixSizeType d1) 
        : MatrixBase<ValueType>(d1), MatrixIndex1<ValueType>(this->getRawBuffer(), d1){}
    
    Matrix1DBase(const std::initializer_list<ValueType> &ls) 
        : MatrixBase<ValueType>(ls), MatrixIndex1<ValueType>(this->getRawBuffer(), ls.size()){}

    Matrix1DBase(ConstPointer data, ConstMatrixSizeType d1)
        : MatrixBase<ValueType>(data, d1), MatrixIndex1<ValueType>(this->getRawBuffer(), d1){}

    Matrix1DBase(const MatrixDataType &vec)
        :MatrixBase<ValueType>(vec), MatrixIndex1<ValueType>(this->getRawBuffer(), vec.size()){}

    Matrix1DBase(const MatrixIndex1<ValueType> &index)
        : MatrixBase<ValueType>(index.m_pstart, index.d1), MatrixIndex1<ValueType>(index){}
};