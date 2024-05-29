#pragma once
#include "DynamicMatrixIndex.hpp"
#include <algorithm>
#include <initializer_list>
#include <iterator>


template<class T>
struct MatrixBase{
public:
    using ValueType = DynamicMatrixTraits<T>::ValueType;
    using ConstValueType = DynamicMatrixTraits<T>::ConstValueType;
    using Pointer = DynamicMatrixTraits<T>::Pointer;
    using ConstPointer = DynamicMatrixTraits<T>::ConstPointer;
    using Reference = DynamicMatrixTraits<T>::Reference;
    using ConstReference = DynamicMatrixTraits<T>::ConstReference;
    using MatrixDataType = DynamicMatrixTraits<T>::MatrixDataType;

public:
    MatrixBase(ConstMatrixSizeType sz){
        m_data.reserve(sz);
    }

    MatrixBase(const std::initializer_list<ValueType> &ls) 
        : m_data(ls){}

    MatrixBase(ConstPointer data, ConstMatrixSizeType size){
        m_data.reserve(size);
        std::copy_n(data, size, std::end(m_data));
    }

    MatrixBase(const std::vector<ValueType> &vec){
        m_data = vec;
    }

public:
    ConstPointer getRawBuffer() const {
        return m_data.data();
    }
    
    Pointer getRawBuffer() {
        return m_data.data();
    }

    MatrixSizeType size() const{
        return m_data.size();
    }

    MatrixSizeType size(){
        return m_data.size();
    }
protected:
    MatrixDataType m_data{};
};


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
}; 

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
};

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
};
