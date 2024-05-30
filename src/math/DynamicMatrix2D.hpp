#pragma once
#include "DynamicMatrixBase.hpp"
#include "DynamicMatrix2DIndex.hpp"

template<class T>
class Matrix2DBase : public MatrixBase<T>, public MatrixIndex2<T>{
public:
    using ValueType = DynamicMatrixTraits<T>::ValueType;
    using ConstValueType = DynamicMatrixTraits<T>::ConstValueType;
    using Pointer = DynamicMatrixTraits<T>::Pointer;
    using ConstPointer = DynamicMatrixTraits<T>::ConstPointer;
    using Reference = DynamicMatrixTraits<T>::Reference;
    using ConstReference = DynamicMatrixTraits<T>::ConstReference;
    using MatrixDataType = DynamicMatrixTraits<T>::MatrixDataType;

public:
    Matrix2DBase(ConstMatrixSizeType d2, ConstMatrixSizeType d1) 
        : MatrixBase<ValueType>(d1 * d2), MatrixIndex2<ValueType>(this->getRawBuffer(), d2, d1, d2){} 

    Matrix2DBase(const std::initializer_list<ValueType> &ls, ConstMatrixSizeType d2, ConstMatrixSizeType d1) 
        : MatrixBase<ValueType>(ls), MatrixIndex2<ValueType>(this->getRawBuffer(), d2, d1){
        assert(this->d2 * this->d1 == ls.size()); 
    } 

    Matrix2DBase(ConstPointer data, ConstMatrixSizeType d2, ConstMatrixSizeType d1)
        : MatrixBase<ValueType>(data, d2 * d1), MatrixIndex2<ValueType>(this->getRawBuffer(), d2, d1){}

    Matrix2DBase(const MatrixDataType &vec, ConstMatrixSizeType d2, ConstMatrixSizeType d1)
        : MatrixBase<ValueType>(vec), MatrixIndex2<ValueType>(this->getRawBuffer(), d2, d1){}

    Matrix2DBase(const MatrixIndex2<ValueType> &index)
        : MatrixBase<ValueType>(index.m_pstart, index.d1 * index.d2), MatrixIndex2<ValueType>(index){}
    
    Matrix2DBase(const std::initializer_list<std::initializer_list<ValueType>> &ls)
        : MatrixBase<ValueType>(ls.size() * ls.begin()->size()), MatrixIndex2<ValueType>(this->getRawBuffer(), ls.size(), ls.begin()->size()){
        Pointer p = this->getRawBuffer();
        for(auto &&l : ls){
            std::copy_n(std::begin(l), this->d1, p);
            p += this->d1;
        }
    }
}; 