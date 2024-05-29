#pragma once
#include <algorithm>
#include <initializer_list>
#include <vector>
#include <cassert>
#include "StaticMatrixIndex.hpp"


template<class T, StaticMatrixSizeType di1>
class StaticMatrix1DBase{
public:
    using ValueType = StaticMatrixTraits<T>::ValueType;
    using ConstValueType = StaticMatrixTraits<T>::ConstValueType;
    using Pointer = StaticMatrixTraits<T>::Pointer;
    using ConstPointer = StaticMatrixTraits<T>::ConstPointer;
    using Reference = StaticMatrixTraits<T>::Reference;
    using ConstReference = StaticMatrixTraits<T>::ConstReference;
 
public:
    constexpr static const StaticMatrixSizeType d1 = di1;
    constexpr static const StaticMatrixSizeType Size = d1;

public:
    StaticMatrix1DBase() = default;
    StaticMatrix1DBase(const std::vector<ValueType> &vec){
        assert(vec.size() == size());
        std::copy_n(std::begin(vec), size(), getRawBuffer());
    }
    
    StaticMatrix1DBase(const std::initializer_list<ValueType> &ls){
        assert(ls.size() == size());
        std::copy_n(std::begin(ls), size(), getRawBuffer());
    }
    
    StaticMatrix1DBase(Pointer data){
        std::copy_n(data, size(), getRawBuffer());
    }
    
    StaticMatrix1DBase(const StaticMatrixIndex1<ValueType, d1> mat){
        std::copy_n(mat.m_pdata, size(), getRawBuffer());
    }

    StaticMatrix1DBase(const ConstStaticMatrixIndex1<ValueType, d1> mat){
        std::copy_n(mat.m_pdata, size(), getRawBuffer());
    }

    StaticMatrixSizeType size() const{
        return Size;
    }

    StaticMatrixSizeType size(){
        return Size;
    }

    Pointer getRawBuffer() const{
        return m_pdata;
    }

    Pointer getRawBuffer(){
        return reinterpret_cast<Pointer>(m_pdata);
    }
    
    ValueType operator[](const StaticMatrixSizeType idx) const{
        return m_pdata[idx];
    }

    Reference operator[](const StaticMatrixSizeType idx){
        return m_pdata[idx];
    }
private:
    ValueType m_pdata[d1]{};
};


template<class T, StaticMatrixSizeType di2, StaticMatrixSizeType di1>
class StaticMatrix2DBase{
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
    constexpr static const StaticMatrixSizeType Size = d2 * d1;
public:
    StaticMatrix2DBase() = default;
    StaticMatrix2DBase(const std::vector<ValueType> &vec){
        assert(vec.size() == size());
        std::copy_n(std::begin(vec), size(), getRawBuffer());
    }
    
    StaticMatrix2DBase(const std::initializer_list<ValueType> &ls){
        assert(ls.size() == size());
        std::copy_n(std::begin(ls), size(), getRawBuffer());
    }

    StaticMatrix2DBase(Pointer data){
        std::copy_n(data, size(), getRawBuffer());
    }
    
    StaticMatrix2DBase(const StaticMatrixIndex2<ValueType, d2, d1> &mat){
        std::copy_n(mat.m_pdata, size(), getRawBuffer());
    }

    StaticMatrix2DBase(const ConstStaticMatrixIndex2<ValueType, d2, d1> &mat){
        std::copy_n(mat.m_pdata, size(), getRawBuffer());
    }

    StaticMatrixSizeType size() const{
        return Size;
    }

    StaticMatrixSizeType size(){
        return Size;
    }

    ConstPointer getRawBuffer() const {
        return (ConstPointer)(m_pdata);
    }

    Pointer getRawBuffer(){
        return reinterpret_cast<Pointer>(m_pdata);
    }

    const ConstStaticMatrixIndex1<ValueType, d1> operator[](const StaticMatrixSizeType idx) const {
        return ConstStaticMatrixIndex1<ValueType, d1>(getRawBuffer() + idx * d1);
    }

    StaticMatrixIndex1<ValueType, d1> operator[](const StaticMatrixSizeType idx){
        return StaticMatrixIndex1<ValueType, d1>(getRawBuffer() + idx * d1);
    }
private:
    ValueType m_pdata[d2][d1]{};
};


template<class T, StaticMatrixSizeType di3, StaticMatrixSizeType di2, StaticMatrixSizeType di1>
class StaticMatrix3DBase{
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
    constexpr static const StaticMatrixSizeType Size = d3 * d2 * d1;

public:
    StaticMatrix3DBase() = default;
    StaticMatrix3DBase(const std::vector<ValueType> &vec){
        assert(vec.size() == size());
        std::copy_n(std::begin(vec), size(), getRawBuffer());
    }
    
    StaticMatrix3DBase(const std::initializer_list<ValueType> &ls){
        assert(ls.size() == size());
        std::copy_n(std::begin(ls), size(), getRawBuffer());
    }

    StaticMatrix3DBase(Pointer data){
        std::copy_n(data, size(), getRawBuffer());
    }
    
    StaticMatrix3DBase(const StaticMatrixIndex3<ValueType, d3, d2, d1> &mat){
        std::copy_n(mat.m_pdata, size(), getRawBuffer());
    }

    StaticMatrix3DBase(const ConstStaticMatrixIndex3<ValueType, d3, d2, d1> &mat){
        std::copy_n(mat.m_pdata, size(), getRawBuffer());
    }

    StaticMatrixSizeType size() const{
        return Size;
    }

    StaticMatrixSizeType size(){
        return Size;
    }

    ConstPointer getRawBuffer() const{
        return (ConstPointer)(m_pdata);
    }

    Pointer getRawBuffer(){
        return reinterpret_cast<Pointer>(m_pdata);
    }

    ConstStaticMatrixIndex2<ValueType, d2, d1> operator[](const StaticMatrixSizeType idx) const{
        return ConstStaticMatrixIndex2<ValueType, d2, d1>(getRawBuffer());
    }

    StaticMatrixIndex2<ValueType, d2, d1> operator[](const StaticMatrixSizeType idx){
        return StaticMatrixIndex2<ValueType, d2, d1>(getRawBuffer());
    }
private:
    ValueType m_pdata[d3][d2][d1]{};
};

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
