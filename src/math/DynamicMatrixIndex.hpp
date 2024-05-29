#pragma once
#include <cstdint>
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
        return MatrixIndex2<ValueType>(this->m_pstart + idx * this->d1 * this->d2, this->d2, this->d1);
    }

    MatrixIndex2<ValueType> operator[](ConstMatrixSizeType idx){
        return const_cast<const MatrixIndex3<ValueType>*>(this)->operator[](idx);
    }
};

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