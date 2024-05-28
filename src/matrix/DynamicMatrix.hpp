#pragma once
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <vector>

using MatrixSizeType = std::size_t;
using ConstMatrixSizeType = const std::size_t;

template<class T>
class MatrixIndexBase{
public:
    using ValueType = T;
    using ConstValueType = const T;
    using Pointer = ValueType*;
    using ConstPointer = const ValueType*;
    using Reference = ValueType&;
    using ConstReference = const ValueType&;

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
class Matrix1DBase;

template<class T>
class MatrixIndex1 : public MatrixIndexBase<T>, public MatrixIndex1Size{
public:
    using ValueType = MatrixIndexBase<T>::ValueType;
    using Pointer = MatrixIndexBase<T>::Pointer;
    using ConstPointer = MatrixIndexBase<T>::ConstPointer;
    using Reference = MatrixIndexBase<T>::Reference;
    
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

template<class T>
class Matrix2DBase;

//d1 -> width
template<class T>
class MatrixIndex2 : public MatrixIndexBase<T>, public MatrixIndex2Size{
public:
    using ValueType = MatrixIndexBase<T>::ValueType;
    using Pointer = MatrixIndexBase<T>::Pointer;
    using ConstPointer = MatrixIndexBase<T>::ConstPointer;
    using Reference = ValueType &;

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
class Matrix3DBase;

template<class T>
class MatrixIndex3 : public MatrixIndexBase<T>, public MatrixIndex3Size{
public:
    using ValueType = MatrixIndexBase<T>::ValueType;
    using Pointer = MatrixIndexBase<T>::Pointer;
    using ConstPointer = MatrixIndexBase<T>::ConstPointer;
    using Reference = ValueType &;

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
class Matrix4DBase;

template<class T>
class MatrixIndex4 : public MatrixIndexBase<T>, public MatrixIndex4Size{
public:
    using ValueType = MatrixIndexBase<T>::ValueType;
    using Pointer = MatrixIndexBase<T>::Pointer;
    using ConstPointer = MatrixIndexBase<T>::ConstPointer;
    using Reference = ValueType &;

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

template<class T>
struct MatrixBase{
public:
public:
    using ValueType = T;
    using ConstValueType = const T;
    using Pointer = ValueType*;
    using ConstPointer = const ValueType*;
    using Reference = ValueType&;
    using ConstReference = const ValueType&;
    using MatrixDataType = std::vector<ValueType>;

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
protected:
    MatrixDataType m_data{};
};


template<class T>
class Matrix1DBase : public MatrixBase<T>, public MatrixIndex1<T>{
public:
    using ValueType = MatrixBase<T>::ValueType;
    using ConstValueType = MatrixBase<T>::ConstValueType;
    using Pointer = MatrixBase<T>::Pointer;
    using ConstPointer = MatrixBase<T>::ConstPointer;
    using MatrixDataType  = MatrixBase<T>::MatrixDataType;

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
    using ValueType = MatrixBase<T>::ValueType;
    using ConstValueType = MatrixBase<T>::ConstValueType;
    using Pointer = MatrixBase<T>::Pointer;
    using ConstPointer = MatrixBase<T>::ConstPointer;
    using MatrixDataType  = MatrixBase<T>::MatrixDataType;

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
    using ValueType = MatrixBase<T>::ValueType;
    using ConstValueType = MatrixBase<T>::ConstValueType;
    using Pointer = MatrixBase<T>::Pointer;
    using ConstPointer = MatrixBase<T>::ConstPointer;
    using MatrixDataType  = MatrixBase<T>::MatrixDataType;

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
    using ValueType = MatrixBase<T>::ValueType;
    using ConstValueType = MatrixBase<T>::ConstValueType;
    using Pointer = MatrixBase<T>::Pointer;
    using ConstPointer = MatrixBase<T>::ConstPointer;
    using MatrixDataType  = MatrixBase<T>::MatrixDataType;

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
