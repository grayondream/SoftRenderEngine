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

struct MatrixIndex1Size{
    MatrixIndex1Size(const MatrixSizeType s1)
        : d1(s1){}
    MatrixSizeType d1;
};

struct MatrixIndex2Size : public MatrixIndex1Size{
    MatrixIndex2Size(const MatrixSizeType s2, const MatrixSizeType s1)
        : MatrixIndex1Size(s1), d2(s2){}
    MatrixSizeType d2;
};

struct MatrixIndex3Size : public MatrixIndex2Size{
    MatrixIndex3Size(const MatrixSizeType s3, const MatrixSizeType s2, const MatrixSizeType s1)
        : MatrixIndex2Size(s2, s1), d3(s3){}
    MatrixSizeType d3;
};

struct MatrixIndex4Size : public MatrixIndex3Size{
    MatrixIndex4Size(const MatrixSizeType s4, const MatrixSizeType s3, const MatrixSizeType s2, const MatrixSizeType s1)
        : MatrixIndex3Size(s3, s2, s1), d4(s4){}
    MatrixSizeType d4;
};

template<class T>
class Matrix1DBase;

template<class T>
class MatrixIndex1 : public MatrixIndexBase<T>, public MatrixIndex1Size{
public:
    MatrixIndex1(T* data, const MatrixSizeType d1) 
        : MatrixIndexBase<T>(data), MatrixIndex1Size(d1){ }
    
    T& operator[](const MatrixSizeType idx) const{
        return *(this->m_pstart + idx);
    }

    operator Matrix1DBase<T>(){
        return Matrix1DBase<T>(this->m_pstart, this->d1);
    }
};

template<class T>
class Matrix2DBase;

//d1 -> width
template<class T>
class MatrixIndex2 : public MatrixIndexBase<T>, public MatrixIndex2Size{
public:
    MatrixIndex2(T* data, const MatrixSizeType d2, const MatrixSizeType d1) 
        : MatrixIndexBase<T>(data), MatrixIndex2Size(d2, d1){ }

    MatrixIndex1<T> operator[](const MatrixSizeType idx) const{
        return MatrixIndex1<T>(this->m_pstart + idx * this->d1, this->d1);
    }

    operator Matrix2DBase<T>(){
        return Matrix2DBase<T>(this->m_pstart, this->d2, this->d1);
    }
};

template<class T>
class Matrix3DBase;

template<class T>
class MatrixIndex3 : public MatrixIndexBase<T>, public MatrixIndex3Size{
public:
    MatrixIndex3(T* data, const MatrixSizeType d3, const MatrixSizeType d2, const MatrixSizeType d1) 
        : MatrixIndexBase<T>(data), MatrixIndex3Size(d3, d2, d1){}

    MatrixIndex2<T> operator[](const MatrixSizeType idx) const {
        return MatrixIndex2<T>(this->m_pstart + idx * this->d1 * this->d2, this->d2, this->d1);
    }

    operator Matrix3DBase<T>(){
        return Matrix3DBase<T>(this->m_pstart, this->d3, this->d2, this->d1);
    }
};

template<class T>
class Matrix4DBase;

template<class T>
class MatrixIndex4 : public MatrixIndexBase<T>, public MatrixIndex4Size{
public:
    MatrixIndex4(T* data, const MatrixSizeType d4, const MatrixSizeType d3, const MatrixSizeType d2, const MatrixSizeType d1) 
            : MatrixIndexBase<T>(data), MatrixIndex4Size(d4, d3, d2, d1){ }

    MatrixIndex3<T> operator[](const MatrixSizeType idx) const {
        return MatrixIndex3<T>(this->m_pstart + idx * this->d1 * this->d2 * this->d3, this->d3, this->d2, this->d1);
    }

    operator Matrix4DBase<T>(){
        return Matrix4DBase<T>(this->m_pstart, this->d4, this->d3, this->d2, this->d1);
    }
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
        assert(this->d2 * this->d1 == ls.size()); 
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

    Matrix3DBase(const T* data, const MatrixSizeType d3, const MatrixSizeType d2, const MatrixSizeType d1)
        : MatrixBase<T>(data, d3 * d2 * d1), MatrixIndex3<T>(this->getRawBuffer(), d3, d2, d1){}

    Matrix3DBase(const std::vector<T> &vec, const MatrixSizeType d3, const MatrixSizeType d2, const MatrixSizeType d1)
        : MatrixBase<T>(vec), MatrixIndex3<T>(this->getRawBuffer(), d3, d2, d1){}
};

template<class T>
class Matrix4DBase : public MatrixBase<T>, public MatrixIndex4<T>{
public:
    Matrix4DBase(const MatrixSizeType d4, const MatrixSizeType d3, const MatrixSizeType d2, const MatrixSizeType d1) 
        : MatrixBase<T>(d1 * d2 * d3 * d4), MatrixIndex4<int>(this->getRawBuffer(), d4, d3, d2, d1){ }

    Matrix4DBase(const std::initializer_list<T> &ls, const MatrixSizeType d4, const MatrixSizeType d3, const MatrixSizeType d2, const MatrixSizeType d1)
        : MatrixBase<T>(d4 * d3 * d2 * d1), MatrixIndex4<T>(this->getRawBuffer(), d4, d3, d2, d1){
        assert(ls.size() == d4 * d3 * d2 * d1);
    }
    Matrix4DBase(const T* data, const MatrixSizeType d4, const MatrixSizeType d3, const MatrixSizeType d2, const MatrixSizeType d1) 
        : MatrixBase<T>(data, d1 * d2 * d3 * d4), MatrixIndex4<int>(this->getRawBuffer(), d4, d3, d2, d1){ }

    Matrix4DBase(const std::vector<T> &vec, const MatrixSizeType d4, const MatrixSizeType d3, const MatrixSizeType d2, const MatrixSizeType d1) 
        : MatrixBase<T>(vec), MatrixIndex4<T>(this->getRawBuffer(), d4, d3, d2, d1){ }
};

using Matrix1D = Matrix1DBase<double>;
using Matrix2D = Matrix2DBase<double>;
using Matrix3D = Matrix3DBase<double>;
using Matrix4D = Matrix4DBase<double>;
