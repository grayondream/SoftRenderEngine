#pragma once
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
class MatrixIndex1 : public MatrixIndexBase<T>{
public:
    MatrixIndex1(T* data) : MatrixIndexBase<T>(data){ }

    T& operator[](const MatrixSizeType idx) const{
        return *(this->m_pstart + idx);
    }
};

//d1 -> width
template<class T>
class MatrixIndex2 : public MatrixIndex1<T>{
public:
    MatrixIndex2(T* data, const MatrixSizeType d1) : MatrixIndex1<T>(data), m_d1(d1){ }

    MatrixIndex1<T> operator[](const MatrixSizeType idx){
        return MatrixIndex1<T>(this->m_pstart + idx * this->m_d1);
    }
protected:
    MatrixSizeType m_d1{};
};

template<class T>
class MatrixIndex3 : public MatrixIndex2<T>{
public:
    MatrixIndex3(T* data, const MatrixSizeType d2, const MatrixSizeType d1) : MatrixIndex2<T>(data, d1), m_d2(d2){ }

    MatrixIndex2<T> operator[](const MatrixSizeType idx) {
        return MatrixIndex2<T>(this->m_pstart + idx * this->m_d1 * this->m_d2);
    }
public:
    MatrixSizeType m_d2{};
};

template<class T>
class MatrixIndex4 : public MatrixIndex3<T>{
public:
    MatrixIndex4(T* data, const MatrixSizeType d3, const MatrixSizeType d2, const MatrixSizeType d1) : MatrixIndex3<T>(data, d2, d1), m_d3(d3){}
    MatrixIndex3<T> operator[](const MatrixSizeType idx) {
        return MatrixIndex3<T>(this->m_pstart + idx * this->m_d1 * this->m_d2 * this->m_d3);
    }
public:
    MatrixSizeType m_d3{};
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
public:
    MatrixValueType* getRawBuffer(){
        return m_data.data();
    }

    MatrixSizeType size(){
        return m_data.size();
    }
protected:
    MatrixDataType m_data{};
};


template<class T>
class Matrix1DBase : public MatrixBase<T>, public MatrixIndex1<T>{
public:
    Matrix1DBase(const MatrixSizeType d1) : MatrixBase<T>(d1), MatrixIndex1<T>(this->getRawBuffer()), m_d1(d1){}
public:
    MatrixSizeType m_d1;
};

template<class T>
class Matrix2DBase : public MatrixBase<T>, public MatrixIndex2<T>{
public:
    Matrix2DBase(const MatrixSizeType d2, const MatrixSizeType d1) : MatrixBase<T>(d1 * d2), MatrixIndex2<T>(this->getRawBuffer(), d2, d1), m_d2(d2){}
public:
    MatrixSizeType m_d2{};
};

template<class T>
class Matrix3DBase : public MatrixBase<T>, public MatrixIndex3<T>{
public:
    Matrix3DBase(const MatrixSizeType d3, const MatrixSizeType d2, const MatrixSizeType d1) : MatrixBase<T>(d1 * d2 * d3), MatrixIndex3<T>(this->getRawBuffer(), d3, d2, d1), m_d3(d3){}
public:
    MatrixSizeType m_d3{};
};

template<class T>
class Matrix4DBase : public MatrixBase<T>, public MatrixIndex4<T>{
public:
    Matrix4DBase(const MatrixSizeType d4, const MatrixSizeType d3, const MatrixSizeType d2, const MatrixSizeType d1) :MatrixBase<T>(d1 * d2 * d3 * d4),MatrixIndex4<int>(this->getRawBuffer(), d4, d3, d2, d1), m_d4(d4){
    }
public:
    MatrixSizeType m_d4{};
};

using Matrix1D = Matrix1DBase<double>;
using Matrix2D = Matrix2DBase<double>;
using Matrix3D = Matrix3DBase<double>;
using Matrix4D = Matrix4DBase<double>;
