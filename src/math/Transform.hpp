#pragma once
#include "Matrix/Matrix.hpp"
#include "Vector/Vector.hpp"
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

template<class T>
Matrix4DBase<T> lookAt(
    const Vector3DBase<T> &eyePos,
    const Vector3DBase<T> &center,
    const Vector3DBase<T> &up){
    Vector3DBase<T> f = center - eyePos;
    f.normalize();
    Vector3DBase<T> r = f.mul(up);
    r.normalize();
    Vector3DBase<T> u = r.mul(f);

    Matrix4DBase<T> m(1,1,4,4);
    m.fill(T{});
    m[0][0][0][0] = r.x;
    m[0][0][0][1] = r.y;
    m[0][0][0][2] = r.z;
    m[0][0][1][0] = u.x;
    m[0][0][1][1] = u.y;
    m[0][0][1][2] = u.z;
    m[0][0][2][0] = -f.x;
    m[0][0][2][1] = -f.y;
    m[0][0][2][2] = -f.z;
    m[0][0][3][3] = T{1};

    m[0][0][0][3] = -r.dot(eyePos);
    m[0][0][1][3] = -u.dot(eyePos);
    m[0][0][2][3] = f.dot(eyePos);

    return m;
}

template<class T>
Matrix4DBase<T> perspective(T fovY, T aspect, T nearVal, T farVal){
    T tanHalf = std::tan(fovY / T{2});
    Matrix4DBase<T> m(1,1,4,4);
    m.fill(T{});
    m[0][0][0][0] = T{1} / (aspect * tanHalf);
    m[0][0][1][1] = T{1} / tanHalf;
    m[0][0][2][2] = -(farVal + nearVal) / (farVal - nearVal);
    m[0][0][2][3] = -(T{2} * farVal * nearVal) / (farVal - nearVal);
    m[0][0][3][2] = T{-1};
    return m;
}

template<class T>
Matrix4DBase<T> orthographic(T left, T right, T bottom, T top, T nearVal, T farVal){
    Matrix4DBase<T> m(1,1,4,4);
    m.fill(T{});
    m[0][0][0][0] = T{2} / (right - left);
    m[0][0][1][1] = T{2} / (top - bottom);
    m[0][0][2][2] = T{-2} / (farVal - nearVal);
    m[0][0][0][3] = -(right + left) / (right - left);
    m[0][0][1][3] = -(top + bottom) / (top - bottom);
    m[0][0][2][3] = -(farVal + nearVal) / (farVal - nearVal);
    m[0][0][3][3] = T{1};
    return m;
}

} // namespace SGE::Math
