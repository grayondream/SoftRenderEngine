#pragma once
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <vector>
using MatrixSizeType = std::size_t;
using ConstMatrixSizeType = const std::size_t;

template<class T>
class DynamicMatrixTraits{
public:
    using ValueType = T;
    using ConstValueType = const T;
    using Pointer = T*;
    using ConstPointer = const T*;
    using Reference = T&;
    using ConstReference = const T&;
    using MatrixDataType = std::vector<ValueType>;
};

template<class T>
class MatrixIndexBase{
public:
    using ValueType = DynamicMatrixTraits<T>::ValueType;
    using ConstValueType = DynamicMatrixTraits<T>::ConstValueType;
    using Pointer = DynamicMatrixTraits<T>::Pointer;
    using ConstPointer = DynamicMatrixTraits<T>::ConstPointer;
    using Reference = DynamicMatrixTraits<T>::Reference;
    using ConstReference = DynamicMatrixTraits<T>::ConstReference;

public:
    MatrixIndexBase(Pointer data) : m_pstart(data){ }
    MatrixIndexBase(ConstPointer data) : m_pstart(data){ }

public:
    Pointer m_pstart{};
};


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

struct MatrixIndex1Size{
    MatrixIndex1Size(ConstMatrixSizeType s1)
        : d1(s1){}
    MatrixSizeType d1;
};

struct MatrixIndex2Size : public MatrixIndex1Size{
    MatrixIndex2Size(ConstMatrixSizeType s2, ConstMatrixSizeType s1)
        : MatrixIndex1Size(s1), d2(s2){}
    MatrixSizeType d2;
};

struct MatrixIndex3Size : public MatrixIndex2Size{
    MatrixIndex3Size(ConstMatrixSizeType s3, ConstMatrixSizeType s2, ConstMatrixSizeType s1)
        : MatrixIndex2Size(s2, s1), d3(s3){}
    MatrixSizeType d3;
};

struct MatrixIndex4Size : public MatrixIndex3Size{
    MatrixIndex4Size(ConstMatrixSizeType s4, ConstMatrixSizeType s3, ConstMatrixSizeType s2, ConstMatrixSizeType s1)
        : MatrixIndex3Size(s3, s2, s1), d4(s4){}
    MatrixSizeType d4;
};





