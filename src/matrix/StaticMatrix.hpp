#pragma once
#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <vector>
#include <cassert>

using StaticMatrixSizeType = std::size_t;

template<class T>
class StaticMatrixBase{
public:
    StaticMatrixBase(T *data)
        : m_pdata(data){}
public:
    T* m_pdata{};
};

template<class T, StaticMatrixSizeType di1>
class StaticMatrixIndex1 : public StaticMatrixBase<T>{
public:
    T& operator[](const StaticMatrixSizeType idx){
        return this->m_pdata[idx];
    }
};

template<class T, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class StaticMatrixIndex2 : public StaticMatrixBase<T>{ 
public:
    StaticMatrixIndex1<T, d1> operator[](const StaticMatrixSizeType idx){
        return StaticMatrixIndex1<T, d1>(this->m_pdata + idx * d1);
    }
};

template<class T, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class StaticMatrixIndex3 : public StaticMatrixBase<T>{ 
public:
    StaticMatrixIndex2<T, d2, d1> operator[](const StaticMatrixSizeType idx){
        return { this->m_pdata + idx * d1 * d2};
    }
};

template<class T, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class StaticMatrixIndex4 : public StaticMatrixBase<T>{ 
public:
    StaticMatrixIndex3<T, d3, d2, d1> operator[](const StaticMatrixSizeType idx){
        return { this->m_pdata + idx * d3 * d2 * d1};
    }
};

template<class T, StaticMatrixSizeType di1>
class StaticMatrix1DBase{
public:
    constexpr static const StaticMatrixSizeType d1 = di1;
    constexpr static const StaticMatrixSizeType Size = d1;
public:
    StaticMatrix1DBase() = default;
    StaticMatrix1DBase(const std::vector<T> &vec){
        assert(vec.size() == size());
        std::copy_n(std::begin(vec), size(), getRawBuffer());
    }
    
    StaticMatrix1DBase(const std::initializer_list<T> &ls){
        assert(ls.size() == size());
        std::copy_n(std::begin(ls), size(), getRawBuffer());
    }
    
    StaticMatrix1DBase(T* data){
        std::copy_n(data, size(), getRawBuffer());
    }
    
    StaticMatrix1DBase(const StaticMatrixIndex1<T, d1> mat){
        std::copy_n(mat.m_pdata, size(), getRawBuffer());
    }

    StaticMatrixSizeType size() const{
        return Size;
    }

    T* getRawBuffer() const{
        return m_pdata;
    }

    T* getRawBuffer(){
        return reinterpret_cast<T*>(m_pdata);
    }
    
    T operator[](const StaticMatrixSizeType idx) const{
        return m_pdata[idx];
    }

    T& operator[](const StaticMatrixSizeType idx){
        return m_pdata[idx];
    }
private:
    T m_pdata[d1]{};
};


template<class T, StaticMatrixSizeType di2, StaticMatrixSizeType di1>
class StaticMatrix2DBase{
public:
    constexpr static const StaticMatrixSizeType d1 = di1;
    constexpr static const StaticMatrixSizeType d2 = di2;
    constexpr static const StaticMatrixSizeType Size = d2 * d1;
public:
    StaticMatrix2DBase() = default;
    StaticMatrix2DBase(const std::vector<T> &vec){
        assert(vec.size() == size());
        std::copy_n(std::begin(vec), size(), getRawBuffer());
    }
    
    StaticMatrix2DBase(const std::initializer_list<T> &ls){
        assert(ls.size() == size());
        std::copy_n(std::begin(ls), size(), getRawBuffer());
    }

    StaticMatrix2DBase(T* data){
        std::copy_n(data, size(), getRawBuffer());
    }
    
    StaticMatrix2DBase(const StaticMatrixIndex2<T, d2, d1> &mat){
        std::copy_n(mat.m_pdata, size(), getRawBuffer());
    }

    StaticMatrixSizeType size() const{
        return Size;
    }

    T* getRawBuffer() const {
        return (T*)(m_pdata);
    }

    T* getRawBuffer(){
        return reinterpret_cast<T*>(m_pdata);
    }

    StaticMatrixIndex1<T, d1> operator[](const StaticMatrixSizeType idx) const {
        return StaticMatrixIndex1<T, d1>(getRawBuffer() + idx * d1);
    }
private:
    T m_pdata[d2][d1]{};
};


template<class T, StaticMatrixSizeType di3, StaticMatrixSizeType di2, StaticMatrixSizeType di1>
class StaticMatrix3DBase{
public:
    constexpr static const StaticMatrixSizeType d1 = di1;
    constexpr static const StaticMatrixSizeType d2 = di2;
    constexpr static const StaticMatrixSizeType d3 = di3;
    constexpr static const StaticMatrixSizeType Size = d3 * d2 * d1;
public:
    StaticMatrix3DBase() = default;
    StaticMatrix3DBase(const std::vector<T> &vec){
        assert(vec.size() == size());
        std::copy_n(std::begin(vec), size(), getRawBuffer());
    }
    
    StaticMatrix3DBase(const std::initializer_list<T> &ls){
        assert(ls.size() == size());
        std::copy_n(std::begin(ls), size(), getRawBuffer());
    }

    StaticMatrix3DBase(T* data){
        std::copy_n(data, size(), getRawBuffer());
    }
    
    StaticMatrix3DBase(const StaticMatrixIndex3<T, d3, d2, d1> &mat){
        std::copy_n(mat.m_pdata, size(), getRawBuffer());
    }

    StaticMatrixSizeType size() const{
        return Size;
    }

    T* getRawBuffer() const{
        return (T*)(m_pdata);
    }

    T* getRawBuffer(){
        return reinterpret_cast<T*>(m_pdata);
    }

    StaticMatrixIndex2<T, d2, d1> operator[](const StaticMatrixSizeType idx) const{
        return StaticMatrixIndex2<T, d2, d1>(getRawBuffer());
    }
private:
    T m_pdata[d3][d2][d1]{};
};

template<class T, StaticMatrixSizeType di4, StaticMatrixSizeType di3, StaticMatrixSizeType di2, StaticMatrixSizeType di1>
class StaticMatrix4DBase{
public:
    constexpr static const StaticMatrixSizeType d1 = di1;
    constexpr static const StaticMatrixSizeType d2 = di2;
    constexpr static const StaticMatrixSizeType d3 = di3;
    constexpr static const StaticMatrixSizeType d4 = di4;
    constexpr static const StaticMatrixSizeType Size = d4 * d3 * d2 * d1;
public:
    StaticMatrix4DBase() = default;
    StaticMatrix4DBase(const std::vector<T> &vec){
        assert(vec.size() == size());
        std::copy_n(std::begin(vec), size(), getRawBuffer());
    }
    
    StaticMatrix4DBase(const std::initializer_list<T> &ls){
        assert(ls.size() == size());
        std::copy_n(std::begin(ls), size(), getRawBuffer());
    }

    StaticMatrix4DBase(T* data){
        std::copy_n(data, size(), getRawBuffer());
    }

    StaticMatrix4DBase(const StaticMatrixIndex4<T, d4, d3, d2, d1> &mat){
        std::copy_n(mat.m_pdata, size(), m_pdata);
    }

    StaticMatrixSizeType size() const{
        return Size;
    }

    T* getRawBuffer() const{
        return reinterpret_cast<T*>(m_pdata);
    }

    T* getRawBuffer(){
        return reinterpret_cast<T*>(m_pdata);
    }

    StaticMatrixIndex3<T, d3, d2, d1> operator[](const StaticMatrixSizeType idx) const {
        return StaticMatrixIndex3<T, d3, d2, d1>(getRawBuffer());
    }
private:
    T m_pdata[d4][d3][d2][d1]{};
};
