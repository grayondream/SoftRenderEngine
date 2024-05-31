#pragma once

#pragma once
#include "DynamicMatrixBase.hpp"
#include "DynamicMatrix4DIndex.hpp"

template<class T>
class Matrix4DBase : public MatrixBase<T>, public MatrixIndex4<T>{
public:
    using ValueType = DynamicMatrixTraits<T>::ValueType;
    using ConstValueType = DynamicMatrixTraits<T>::ConstValueType;
    using Pointer = DynamicMatrixTraits<T>::Pointer;
    using ConstPointer = DynamicMatrixTraits<T>::ConstPointer;
    using Reference = DynamicMatrixTraits<T>::Reference;
    using ConstReference = DynamicMatrixTraits<T>::ConstReference;
    using MatrixDataType = DynamicMatrixTraits<T>::MatrixDataType;

public:
    Matrix4DBase(ConstMatrixSizeType d4, ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1) 
        : MatrixBase<ValueType>(d1 * d2 * d3 * d4), MatrixIndex4<ValueType>(this->getRawBuffer(), d4, d3, d2, d1){ }

    Matrix4DBase(const std::initializer_list<ValueType> &ls, ConstMatrixSizeType d4, ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1)
        : MatrixBase<ValueType>(d4 * d3 * d2 * d1), MatrixIndex4<ValueType>(this->getRawBuffer(), d4, d3, d2, d1){
        assert(ls.size() == d4 * d3 * d2 * d1);
    }
    Matrix4DBase(ConstPointer data, ConstMatrixSizeType d4, ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1) 
        : MatrixBase<ValueType>(data, d1 * d2 * d3 * d4), MatrixIndex4<ValueType>(this->getRawBuffer(), d4, d3, d2, d1){ }

    Matrix4DBase(const MatrixDataType &vec, ConstMatrixSizeType d4, ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1) 
        : MatrixBase<ValueType>(vec), MatrixIndex4<ValueType>(this->getRawBuffer(), d4, d3, d2, d1){ }

    Matrix4DBase(const MatrixIndex4<ValueType> &index)
        : MatrixBase<ValueType>(index.m_pstart, index.d1 * index.d2 * index.d3 * index.d4), MatrixIndex4<ValueType>(index){}

    Matrix4DBase(const std::initializer_list<std::initializer_list<std::initializer_list<std::initializer_list<ValueType>>>> &ls)
        :   MatrixBase<ValueType>(ls.size() * ls.begin()->size() * ls.begin()->begin().size() * ls.begin()->begin()->begin()->size()),
            MatrixIndex4<ValueType>(this->getRawBuffer(), ls.size(), ls.begin()->size(), ls.begin()->begin()->size(), ls.begin()->begin()->begin()->size()){
        Pointer p = this->getRawBuffer();
        for(auto && c : ls){
            for(auto &&r : c){
                for(auto && l : r){
                    std::copy_n(std::begin(l), this->d1, p);
                    p += this->d1;
                }
            }
        }
    }
};
