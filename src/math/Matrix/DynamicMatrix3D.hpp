#pragma once
#include "DynamicMatrixBase.hpp"
#include "DynamicMatrix3DIndex.hpp"


template<class T> class Matrix3DBase : public MatrixBase<T>, public MatrixIndex3<T>{
public:
    using ValueType = DynamicMatrixTraits<T>::ValueType;
    using ConstValueType = DynamicMatrixTraits<T>::ConstValueType;
    using Pointer = DynamicMatrixTraits<T>::Pointer;
    using ConstPointer = DynamicMatrixTraits<T>::ConstPointer;
    using Reference = DynamicMatrixTraits<T>::Reference;
    using ConstReference = DynamicMatrixTraits<T>::ConstReference;
    using MatrixDataType = DynamicMatrixTraits<T>::MatrixDataType;

public:
    Matrix3DBase(ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1) 
    : MatrixBase<ValueType>(d1 * d2 * d3), MatrixIndex3<ValueType>(this->getRawBuffer(), d3, d2, d1){}

    Matrix3DBase(const std::initializer_list<ValueType> &ls, ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1)
        : MatrixBase<ValueType>(ls), MatrixIndex3<ValueType>(this->getRawBuffer(), d3, d2, d1){
        assert(d3 * d2 * d1 == ls.size());
    }

    Matrix3DBase(ConstPointer data, ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1)
        : MatrixBase<ValueType>(data, d3 * d2 * d1), MatrixIndex3<ValueType>(this->getRawBuffer(), d3, d2, d1){}

    Matrix3DBase(const MatrixDataType &vec, ConstMatrixSizeType d3, ConstMatrixSizeType d2, ConstMatrixSizeType d1)
        : MatrixBase<ValueType>(vec), MatrixIndex3<ValueType>(this->getRawBuffer(), d3, d2, d1){}

    Matrix3DBase(const MatrixIndex3<ValueType> &index)
        : MatrixBase<ValueType>(index.m_pstart, index.d1 * index.d2 * index.d3), MatrixIndex3<ValueType>(index){}

    Matrix3DBase(const std::initializer_list<std::initializer_list<std::initializer_list<ValueType>>> &ls)
        :   MatrixBase<ValueType>(ls.size() * ls.begin()->size() * ls.begin()->begin()->size()),
            MatrixIndex3<ValueType>(this->getRawBuffer(), ls.size(), ls.begin()->size(), ls.begin()->begin()->size()){
        Pointer p = this->getRawBuffer();
        for(auto &&c : ls){
            for(auto && r : c){
                std::copy_n(std::begin(r), this->d1, p);
                p += this->d1;
            }
        }
    }
};