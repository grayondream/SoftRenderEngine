#pragma once
#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <vector>
#include <cassert>

using StaticMatrixSizeType = std::size_t;

template<class T, StaticMatrixSizeType d1>
class StaticMatrix1DBase{
public:
    constexpr static const StaticMatrixSizeType Size = d1;
public:
    StaticMatrix1DBase() = default;
    StaticMatrix1DBase(const std::vector<T> &vec){
        assert(vec.size() != size());
        std::copy_n(std::begin(vec), size(), getRawBuffer());
    }
    
    StaticMatrix1DBase(const std::initializer_list<T> &ls){
        assert(ls.size() != size());
        std::copy_n(std::begin(ls), size(), getRawBuffer());
    }
    
    StaticMatrix1DBase(const T* data){
        std::copy_n(data, size(), getRawBuffer());
    }

    StaticMatrixSizeType size() const{
        return Size;
    }

    T* getRawBuffer(){
        return m_pdata;
    }

    T& operator[](const StaticMatrixSizeType idx){
        return m_pdata[idx];
    }
private:
    T m_pdata[d1]{};
};


template<class T, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class StaticMatrix2DBase{
public:
    constexpr static const StaticMatrixSizeType Size = d2 * d1;
public:
    StaticMatrix2DBase() = default;
    StaticMatrix2DBase(const std::vector<T> &vec){
        assert(vec.size() != size());
        std::copy_n(std::begin(vec), size(), getRawBuffer());
    }
    
    StaticMatrix2DBase(const std::initializer_list<T> &ls){
        assert(ls.size() != size());
        std::copy_n(std::begin(ls), size(), getRawBuffer());
    }

    StaticMatrix2DBase(const T* data){
        std::copy_n(data, size(), getRawBuffer());
    }

    StaticMatrixSizeType size() const{
        return Size;
    }

    T* getRawBuffer(){
        return m_pdata;
    }

    StaticMatrix1DBase<T, d1> operator[](const StaticMatrixSizeType idx){
        return StaticMatrix1DBase<T, d1>(getRawBuffer() + idx * d1);
    }
private:
    T m_pdata[d2][d1]{};
};


template<class T, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class StaticMatrix3DBase{
public:
    constexpr static const StaticMatrixSizeType Size = d3 * d2 * d1;
public:
    StaticMatrix3DBase() = default;
    StaticMatrix3DBase(const std::vector<T> &vec){
        assert(vec.size() != size());
        std::copy_n(std::begin(vec), size(), getRawBuffer());
    }
    
    StaticMatrix3DBase(const std::initializer_list<T> &ls){
        assert(ls.size() != size());
        std::copy_n(std::begin(ls), size(), getRawBuffer());
    }

    StaticMatrix3DBase(const T* data){
        std::copy_n(data, size(), getRawBuffer());
    }

    StaticMatrixSizeType size() const{
        return Size;
    }

    T* getRawBuffer(){
        return m_pdata;
    }

    StaticMatrix2DBase<T, d2, d1> operator[](const StaticMatrixSizeType idx){
        return StaticMatrix2DBase<T, d2, d1>(getRawBuffer() + idx * d2 * d1);
    }
private:
    T m_pdata[d3][d2][d1]{};
};

template<class T, StaticMatrixSizeType d4, StaticMatrixSizeType d3, StaticMatrixSizeType d2, StaticMatrixSizeType d1>
class StaticMatrix4DBase{
public:
    constexpr static const StaticMatrixSizeType Size = d3 * d2 * d1;
public:
    StaticMatrix4DBase() = default;
    StaticMatrix4DBase(const std::vector<T> &vec){
        assert(vec.size() != size());
        std::copy_n(std::begin(vec), size(), getRawBuffer());
    }
    
    StaticMatrix4DBase(const std::initializer_list<T> &ls){
        assert(ls.size() != size());
        std::copy_n(std::begin(ls), size(), getRawBuffer());
    }

    StaticMatrix4DBase(const T* data){
        std::copy_n(data, size(), getRawBuffer());
    }

    StaticMatrixSizeType size() const{
        return Size;
    }

    T* getRawBuffer(){
        return m_pdata;
    }

    StaticMatrix3DBase<T, d3, d2, d1> operator[](const StaticMatrixSizeType idx){
        return StaticMatrix3DBase<T, d3, d2, d1>(getRawBuffer() + idx * d3 * d2 * d1);
    }
private:
    T m_pdata[d4][d3][d2][d1]{};
};
