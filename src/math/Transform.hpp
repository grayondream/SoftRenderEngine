#pragma once
#include "Matrix/Matrix.hpp"
#include <cmath>

namespace SGE::Math {

template<class T>
Matrix4DBase<T> translation(T x, T y, T z){
    Matrix4DBase<T> m(1,1,4,4);
    m.eye();
    m[0][0][0][3] = x;
    m[0][0][1][3] = y;
    m[0][0][2][3] = z;
    return m;
}

template<class T>
Matrix4DBase<T> rotationX(T angle){
    Matrix4DBase<T> m(1,1,4,4);
    m.eye();
    T c = std::cos(angle);
    T s = std::sin(angle);
    m[0][0][1][1] = c;
    m[0][0][1][2] = -s;
    m[0][0][2][1] = s;
    m[0][0][2][2] = c;
    return m;
}

template<class T>
Matrix4DBase<T> rotationY(T angle){
    Matrix4DBase<T> m(1,1,4,4);
    m.eye();
    T c = std::cos(angle);
    T s = std::sin(angle);
    m[0][0][0][0] = c;
    m[0][0][0][2] = s;
    m[0][0][2][0] = -s;
    m[0][0][2][2] = c;
    return m;
}

template<class T>
Matrix4DBase<T> rotationZ(T angle){
    Matrix4DBase<T> m(1,1,4,4);
    m.eye();
    T c = std::cos(angle);
    T s = std::sin(angle);
    m[0][0][0][0] = c;
    m[0][0][0][1] = -s;
    m[0][0][1][0] = s;
    m[0][0][1][1] = c;
    return m;
}

template<class T>
Matrix4DBase<T> scale(T sx, T sy, T sz){
    Matrix4DBase<T> m(1,1,4,4);
    m.fill(T{});
    m[0][0][0][0] = sx;
    m[0][0][1][1] = sy;
    m[0][0][2][2] = sz;
    m[0][0][3][3] = T{1};
    return m;
}

} // namespace SGE::Math
