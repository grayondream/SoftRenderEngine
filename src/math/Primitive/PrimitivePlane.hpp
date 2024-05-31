#pragma once
#include "Vector/Vector.hpp"

template<class T, template<typename> class VectorBase = Vector3DBase, class Line = VectorBase<T>, class Point = VectorBase<T>>
struct PrimitivePlaneBase{
    Point p0;           //平面上的点，通过一个点和法线来确定一个平面
    Line n;             //平面的法线
};