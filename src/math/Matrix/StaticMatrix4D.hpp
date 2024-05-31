#pragma once
#include <algorithm>
#include <initializer_list>
#include <vector>
#include <cassert>
#include "StaticMatrixIndex4.hpp"

template<class T, StaticMatrixSizeType di4, StaticMatrixSizeType di3, StaticMatrixSizeType di2, StaticMatrixSizeType di1>
class StaticMatrix4DBase{
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;
    using ConstValueType = StaticMatrixTraits<T>::ConstValueType;
    using Pointer = StaticMatrixTraits<T>::Pointer;
    using ConstPointer = StaticMatrixTraits<T>::ConstPointer;
    using Reference = StaticMatrixTraits<T>::Reference;
    using ConstReference = StaticMatrixTraits<T>::ConstReference;
 
public:
    constexpr static const StaticMatrixSizeType d1 = di1;
    constexpr static const StaticMatrixSizeType d2 = di2;
    constexpr static const StaticMatrixSizeType d3 = di3;
    constexpr static const StaticMatrixSizeType d4 = di4;
    constexpr static const StaticMatrixSizeType Size = d4 * d3 * d2 * d1;

public:
    StaticMatrix4DBase() = default;
    StaticMatrix4DBase(const std::vector<ValueType> &vec){
        assert(vec.size() == size());
        std::copy_n(std::begin(vec), size(), getRawBuffer());
    }
    
    StaticMatrix4DBase(const std::initializer_list<ValueType> &ls){
        assert(ls.size() == size());
        std::copy_n(std::begin(ls), size(), getRawBuffer());
    }

    StaticMatrix4DBase(Pointer data){
        std::copy_n(data, size(), getRawBuffer());
    }

    StaticMatrix4DBase(const StaticMatrixIndex4<ValueType, d4, d3, d2, d1> &mat){
        std::copy_n(mat.m_pdata, size(), m_pdata);
    }
    
    StaticMatrix4DBase(const ConstStaticMatrixIndex4<ValueType, d4, d3, d2, d1> &mat){
        std::copy_n(mat.m_pdata, size(), m_pdata);
    }

    StaticMatrix4DBase(const std::initializer_list<std::initializer_list<std::initializer_list<std::initializer_list<ValueType>>>> &ls){
        assert(ls.size() == d3);
        assert(ls.begin()->size() == d2);
        assert(ls.begin()->begin()->size() == d1);
        Pointer p = getRawBuffer();
        for(auto && c : ls){
            for(auto &&r : c){
                for(auto && l : r){
                    std::copy_n(std::begin(l), d1, p);
                    p += d1;
                }
            }
        }
    }

    StaticMatrixSizeType size() const{
        return Size;
    }

    StaticMatrixSizeType size(){
        return Size;
    }

    ConstPointer getRawBuffer() const{
        return reinterpret_cast<ConstPointer>(m_pdata);
    }

    Pointer getRawBuffer(){
        return reinterpret_cast<Pointer>(m_pdata);
    }

    StaticMatrixIndex3<ValueType, d3, d2, d1> operator[](const StaticMatrixSizeType idx) {
        return StaticMatrixIndex3<ValueType, d3, d2, d1>(getRawBuffer());
    }

    ConstStaticMatrixIndex3<ValueType, d3, d2, d1> operator[](const StaticMatrixSizeType idx) const {
        return ConstStaticMatrixIndex3<ValueType, d3, d2, d1>(getRawBuffer());
    }
private:
    ValueType m_pdata[d4][d3][d2][d1]{};
};
