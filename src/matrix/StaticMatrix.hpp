#pragma once
#include <algorithm>
#include <initializer_list>
#include <vector>
#include <cassert>

using StaticMatrixSizeType = std::size_t;

template<class T>
class StaticMatrixBase{
public:
    using ValueType = T;
    using ConstValueType = const T;
    using Pointer = T*;
    using ConstPointer = const T*;
    using Reference = T&;
    using ConstReference = const T&;

public:
    StaticMatrixBase(Pointer data)
        : m_pdata(data){}
public:
    Pointer m_pdata{};
};

template<class T, StaticMatrixSizeType di1>
class StaticMatrixIndex1 : public StaticMatrixBase<T>{
public:
    using ValueType = StaticMatrixBase<T>::ValueType;
    using ConstValueType = StaticMatrixBase<T>::ConstValueType;
    using Pointer = StaticMatrixBase<T>::Pointer;
    using ConstPointer = StaticMatrixBase<T>::ConstPointer;
    using Reference = StaticMatrixBase<T>::Reference;
    using ConstReference = StaticMatrixBase<T>::ConstReference;

public:
    ValueType& operator[](const StaticMatrixSizeType idx){
        return this->m_pdata[idx];
    }

    ValueType operator[](const StaticMatrixSizeType idx) const {
        return this->m_pdata[idx];
    }
};

template<class T, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class StaticMatrixIndex2 : public StaticMatrixBase<T>{ 
public:
    using ValueType = StaticMatrixBase<T>::ValueType;
    using ConstValueType = StaticMatrixBase<T>::ConstValueType;
    using Pointer = StaticMatrixBase<T>::Pointer;
    using ConstPointer = StaticMatrixBase<T>::ConstPointer;
    using Reference = StaticMatrixBase<T>::Reference;
    using ConstReference = StaticMatrixBase<T>::ConstReference;

public:
    StaticMatrixIndex1<ValueType, d1> operator[](const StaticMatrixSizeType idx){
        return StaticMatrixIndex1<ValueType, d1>(this->m_pdata + idx * d1);
    }

    StaticMatrixIndex1<ValueType, d1> operator[](const StaticMatrixSizeType idx) const{
        return StaticMatrixIndex1<ValueType, d1>(this->m_pdata + idx * d1);
    }
};

template<class T, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class StaticMatrixIndex3 : public StaticMatrixBase<T>{ 
public:
    using ValueType = StaticMatrixBase<T>::ValueType;
    using ConstValueType = StaticMatrixBase<T>::ConstValueType;
    using Pointer = StaticMatrixBase<T>::Pointer;
    using ConstPointer = StaticMatrixBase<T>::ConstPointer;
    using Reference = StaticMatrixBase<T>::Reference;
    using ConstReference = StaticMatrixBase<T>::ConstReference;

public:
    StaticMatrixIndex2<ValueType, d2, d1> operator[](const StaticMatrixSizeType idx){
        return { this->m_pdata + idx * d1 * d2};
    }

    StaticMatrixIndex2<ValueType, d2, d1> operator[](const StaticMatrixSizeType idx) const {
        return { this->m_pdata + idx * d1 * d2};
    }
};

template<class T, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class StaticMatrixIndex4 : public StaticMatrixBase<T>{ 
public:
    using ValueType = StaticMatrixBase<T>::ValueType;
    using ConstValueType = StaticMatrixBase<T>::ConstValueType;
    using Pointer = StaticMatrixBase<T>::Pointer;
    using ConstPointer = StaticMatrixBase<T>::ConstPointer;
    using Reference = StaticMatrixBase<T>::Reference;
    using ConstReference = StaticMatrixBase<T>::ConstReference;

public:
    StaticMatrixIndex3<ValueType, d3, d2, d1> operator[](const StaticMatrixSizeType idx){
        return { this->m_pdata + idx * d3 * d2 * d1};
    }

    StaticMatrixIndex3<ValueType, d3, d2, d1> operator[](const StaticMatrixSizeType idx) const{
        return { this->m_pdata + idx * d3 * d2 * d1};
    }
};

template<class T, StaticMatrixSizeType di1>
class StaticMatrix1DBase{
public:
    using ValueType = StaticMatrixBase<T>::ValueType;
    using ConstValueType = StaticMatrixBase<T>::ConstValueType;
    using Pointer = StaticMatrixBase<T>::Pointer;
    using ConstPointer = StaticMatrixBase<T>::ConstPointer;
    using Reference = StaticMatrixBase<T>::Reference;
    using ConstReference = StaticMatrixBase<T>::ConstReference;
 
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
    using ValueType = StaticMatrixBase<T>::ValueType;
    using ConstValueType = StaticMatrixBase<T>::ConstValueType;
    using Pointer = StaticMatrixBase<T>::Pointer;
    using ConstPointer = StaticMatrixBase<T>::ConstPointer;
    using Reference = StaticMatrixBase<T>::Reference;
    using ConstReference = StaticMatrixBase<T>::ConstReference;
 
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

    StaticMatrixSizeType size() const{
        return Size;
    }

    StaticMatrixSizeType size(){
        return Size;
    }

    Pointer getRawBuffer() const {
        return (Pointer)(m_pdata);
    }

    Pointer getRawBuffer(){
        return reinterpret_cast<Pointer>(m_pdata);
    }

    StaticMatrixIndex1<ValueType, d1> operator[](const StaticMatrixSizeType idx) const {
        return StaticMatrixIndex1<ValueType, d1>(getRawBuffer() + idx * d1);
    }
private:
    ValueType m_pdata[d2][d1]{};
};


template<class T, StaticMatrixSizeType di3, StaticMatrixSizeType di2, StaticMatrixSizeType di1>
class StaticMatrix3DBase{
public:
    using ValueType = StaticMatrixBase<T>::ValueType;
    using ConstValueType = StaticMatrixBase<T>::ConstValueType;
    using Pointer = StaticMatrixBase<T>::Pointer;
    using ConstPointer = StaticMatrixBase<T>::ConstPointer;
    using Reference = StaticMatrixBase<T>::Reference;
    using ConstReference = StaticMatrixBase<T>::ConstReference;
 
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

    StaticMatrixSizeType size() const{
        return Size;
    }

    StaticMatrixSizeType size(){
        return Size;
    }

    Pointer getRawBuffer() const{
        return (Pointer)(m_pdata);
    }

    Pointer getRawBuffer(){
        return reinterpret_cast<Pointer>(m_pdata);
    }

    StaticMatrixIndex2<ValueType, d2, d1> operator[](const StaticMatrixSizeType idx) const{
        return StaticMatrixIndex2<ValueType, d2, d1>(getRawBuffer());
    }
private:
    ValueType m_pdata[d3][d2][d1]{};
};

template<class T, StaticMatrixSizeType di4, StaticMatrixSizeType di3, StaticMatrixSizeType di2, StaticMatrixSizeType di1>
class StaticMatrix4DBase{
public:
    using ValueType = StaticMatrixBase<T>::ValueType;
    using ConstValueType = StaticMatrixBase<T>::ConstValueType;
    using Pointer = StaticMatrixBase<T>::Pointer;
    using ConstPointer = StaticMatrixBase<T>::ConstPointer;
    using Reference = StaticMatrixBase<T>::Reference;
    using ConstReference = StaticMatrixBase<T>::ConstReference;
 
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
    
    StaticMatrixSizeType size() const{
        return Size;
    }

    StaticMatrixSizeType size(){
        return Size;
    }

    Pointer getRawBuffer() const{
        return reinterpret_cast<Pointer>(m_pdata);
    }

    Pointer getRawBuffer(){
        return reinterpret_cast<Pointer>(m_pdata);
    }

    StaticMatrixIndex3<ValueType, d3, d2, d1> operator[](const StaticMatrixSizeType idx) const {
        return StaticMatrixIndex3<ValueType, d3, d2, d1>(getRawBuffer());
    }
private:
    ValueType m_pdata[d4][d3][d2][d1]{};
};
