#pragma once
#include <algorithm>
#include <initializer_list>
#include <vector>

using MatrixSizeType = std::size_t;

template<class T>
class MatrixIndexBase{
public:
    MatrixIndexBase(T *data) : m_pstart(data){ }

protected:
    T *m_pstart{};
};

template<class T>
class Matrix1DBase;

template<class T>
class MatrixIndex1 : public MatrixIndexBase<T>{
public:
    MatrixIndex1(T* data, const MatrixSizeType d1) 
        : MatrixIndexBase<T>(data), m_d1(d1){ }
    
    T& operator[](const MatrixSizeType idx) const{
        return *(this->m_pstart + idx);
    }

    operator Matrix1DBase<T>(){
        return Matrix1DBase<T>(this->m_pstart, this->m_d1);
    }
public:
    MatrixSizeType m_d1{};
};

//d1 -> width
template<class T>
class MatrixIndex2 : public MatrixIndex1<T>{
public:
    MatrixIndex2(T* data, const MatrixSizeType d2, const MatrixSizeType d1) 
        : MatrixIndex1<T>(data, d1), m_d2(d2){ }

    MatrixIndex1<T> operator[](const MatrixSizeType idx) const{
        return MatrixIndex1<T>(this->m_pstart + idx * this->m_d1, this->m_d1);
    }
public:
    MatrixSizeType m_d2{};
};

template<class T>
class MatrixIndex3 : public MatrixIndex2<T>{
public:
    MatrixIndex3(T* data, const MatrixSizeType d3, const MatrixSizeType d2, const MatrixSizeType d1) 
        : MatrixIndex2<T>(data, d2, d1), m_d3(d3){ }

    MatrixIndex2<T> operator[](const MatrixSizeType idx) const {
        return MatrixIndex2<T>(this->m_pstart + idx * this->m_d1 * this->m_d2);
    }
public:
    MatrixSizeType m_d3{};
};

template<class T>
class MatrixIndex4 : public MatrixIndex3<T>{
public:
    MatrixIndex4(T* data, const MatrixSizeType d4, const MatrixSizeType d3, const MatrixSizeType d2, const MatrixSizeType d1) 
            : MatrixIndex3<T>(data, d3, d2, d1), m_d4(d4){}

    MatrixIndex3<T> operator[](const MatrixSizeType idx) const {
        return MatrixIndex3<T>(this->m_pstart + idx * this->m_d1 * this->m_d2 * this->m_d3);
    }
public:
    MatrixSizeType m_d4{};
};

template<class T>
struct MatrixBase{
public:
    using MatrixValueType = T;
    using MatrixDataType = std::vector<MatrixValueType>;
public:
    MatrixBase(const MatrixSizeType sz){
        m_data.reserve(sz);
    }

    MatrixBase(const std::initializer_list<T> &ls) 
        : m_data(ls){}

    MatrixBase(const T* data, const std::size_t size){
        m_data.reserve(size);
        std::copy_n(data, size, std::end(m_data));
    }

    MatrixBase(const std::vector<T> &vec){
        m_data = vec;
    }
public:
    MatrixValueType* getRawBuffer(){
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
    Matrix1DBase(const MatrixSizeType d1) 
        : MatrixBase<T>(d1), MatrixIndex1<T>(this->getRawBuffer(), d1){}
    
    Matrix1DBase(const std::initializer_list<T> &ls) 
        : MatrixBase<T>(ls), MatrixIndex1<T>(this->getRawBuffer(), ls.size()){}

    Matrix1DBase(const T* data, const MatrixSizeType d1)
        : MatrixBase<T>(data, d1), MatrixIndex1<T>(this->getRawBuffer(), d1){}

    Matrix1DBase(const std::vector<T> &vec)
        :MatrixBase<T>(vec), MatrixIndex1<T>(this->getRawBuffer(), vec.size()){}
};

template<class T>
class Matrix2DBase : public MatrixBase<T>, public MatrixIndex2<T>{
public:
    Matrix2DBase(const MatrixSizeType d2, const MatrixSizeType d1) 
        : MatrixBase<T>(d1 * d2), MatrixIndex2<T>(this->getRawBuffer(), d2, d1, d2){} 

    Matrix2DBase(const std::initializer_list<T> &ls, const MatrixSizeType d2, const MatrixSizeType d1) 
        : MatrixBase<T>(ls), MatrixIndex2<T>(this->getRawBuffer(), d2, d1){
        assert(this->m_d2 * this->m_d1 == ls.size()); 
    } 

    Matrix2DBase(const T* data, const MatrixSizeType d2, const MatrixSizeType d1)
        : MatrixBase<T>(data, d2 * d1), MatrixIndex2<T>(this->getRawBuffer(), d2, d1){}

    Matrix2DBase(const std::vector<T> &vec, const MatrixSizeType d2, const MatrixSizeType d1)
        : MatrixBase<T>(vec), MatrixIndex2<T>(this->getRawBuffer(), d2, d1){}
}; 

template<class T> class Matrix3DBase : public MatrixBase<T>, public MatrixIndex3<T>{
public:
    Matrix3DBase(const MatrixSizeType d3, const MatrixSizeType d2, const MatrixSizeType d1) 
    : MatrixBase<T>(d1 * d2 * d3), MatrixIndex3<T>(this->getRawBuffer(), d3, d2, d1){}

    Matrix3DBase(const std::initializer_list<T> &ls, const MatrixSizeType d3, const MatrixSizeType d2, const MatrixSizeType d1)
        : MatrixBase<T>(ls), MatrixIndex3<T>(this->getRawBuffer(), d3, d2, d1){
        assert(d3 * d2 * d1 == ls.size());
    }
};

template<class T>
class Matrix4DBase : public MatrixBase<T>, public MatrixIndex4<T>{
public:
    Matrix4DBase(const MatrixSizeType d4, const MatrixSizeType d3, const MatrixSizeType d2, const MatrixSizeType d1) 
        : MatrixBase<T>(d1 * d2 * d3 * d4),MatrixIndex4<int>(this->getRawBuffer(), d4, d3, d2, d1){ }

    Matrix4DBase(const std::initializer_list<T> &ls, const MatrixSizeType d4, const MatrixSizeType d3, const MatrixSizeType d2, const MatrixSizeType d1)
        : MatrixBase<T>(d4 * d3 * d2 * d1), MatrixIndex4<T>(this->getRawBuffer(), d4, d3, d2, d1){
        assert(ls.size() == d4 * d3 * d2 * d1);
    }
};

using Matrix1D = Matrix1DBase<double>;
using Matrix2D = Matrix2DBase<double>;
using Matrix3D = Matrix3DBase<double>;
using Matrix4D = Matrix4DBase<double>;
