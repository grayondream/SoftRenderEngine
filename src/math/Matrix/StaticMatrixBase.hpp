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
