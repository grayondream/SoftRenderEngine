#pragma once
#include <cassert>

using StaticMatrixSizeType = std::size_t;

template<class T>
class StaticMatrixTraits{
public:
    using ValueType = T;
    using ConstValueType = const T;
    using Pointer = T*;
    using ConstPointer = const T*;
    using Reference = T&;
    using ConstReference = const T&;
};

template<class T>
class StaticMatrixIndexBase{
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;
    using ConstValueType = StaticMatrixTraits<T>::ConstValueType;
    using Pointer = StaticMatrixTraits<T>::Pointer;
    using ConstPointer = StaticMatrixTraits<T>::ConstPointer;
    using Reference = StaticMatrixTraits<T>::Reference;
    using ConstReference = StaticMatrixTraits<T>::ConstReference;

public:
    StaticMatrixIndexBase(Pointer data)
        : m_pdata(data){}

public:
    Pointer m_pdata{};
};

template<class T, StaticMatrixSizeType di1>
class StaticMatrixIndex1 : public StaticMatrixIndexBase<T>{
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;

public:
    ValueType& operator[](const StaticMatrixSizeType idx){
        return this->m_pdata[idx];
    }
};

template<class T, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class StaticMatrixIndex2 : public StaticMatrixIndexBase<T>{ 
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;

public:
    StaticMatrixIndex1<ValueType, d1> operator[](const StaticMatrixSizeType idx){
        return StaticMatrixIndex1<ValueType, d1>(this->m_pdata + idx * d1);
    }
};

template<class T, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class StaticMatrixIndex3 : public StaticMatrixIndexBase<T>{ 
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;

public:
    StaticMatrixIndex2<ValueType, d2, d1> operator[](const StaticMatrixSizeType idx){
        return { this->m_pdata + idx * d1 * d2};
    }
};

template<class T, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class StaticMatrixIndex4 : public StaticMatrixIndexBase<T>{ 
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;

public:
    StaticMatrixIndex3<ValueType, d3, d2, d1> operator[](const StaticMatrixSizeType idx){
        return { this->m_pdata + idx * d3 * d2 * d1};
    }
};

//const
template<class T>
class ConstStaticMatrixIndexBase{
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;
    using ConstValueType = StaticMatrixTraits<T>::ConstValueType;
    using Pointer = StaticMatrixTraits<T>::Pointer;
    using ConstPointer = StaticMatrixTraits<T>::ConstPointer;
    using Reference = StaticMatrixTraits<T>::Reference;
    using ConstReference = StaticMatrixTraits<T>::ConstReference;

public:
    ConstStaticMatrixIndexBase(ConstPointer data)
        : m_pdata(data){}

public:
    ConstPointer m_pdata{};
};

template<class T, StaticMatrixSizeType di1>
class ConstStaticMatrixIndex1 : public ConstStaticMatrixIndexBase<T>{
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;

public:
    ValueType operator[](const StaticMatrixSizeType idx) const {
        return this->m_pdata[idx];
    }
};

template<class T, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class ConstStaticMatrixIndex2 : public ConstStaticMatrixIndexBase<T>{ 
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;

public:
    ConstStaticMatrixIndex1<ValueType, d1> operator[](const StaticMatrixSizeType idx) const{
        return ConstStaticMatrixIndex1<ValueType, d1>(this->m_pdata + idx * d1);
    }
};

template<class T, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class ConstStaticMatrixIndex3 : public ConstStaticMatrixIndexBase<T>{ 
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;

public:
    ConstStaticMatrixIndex2<ValueType, d2, d1> operator[](const StaticMatrixSizeType idx) const {
        return { this->m_pdata + idx * d1 * d2};
    }
};

template<class T, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class ConstStaticMatrixIndex4 : public ConstStaticMatrixIndexBase<T>{ 
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;

public:
    ConstStaticMatrixIndex3<ValueType, d3, d2, d1> operator[](const StaticMatrixSizeType idx) const{
        return { this->m_pdata + idx * d3 * d2 * d1};
    }
};