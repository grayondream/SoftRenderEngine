#pragma once
#include "Vector/Vector.hpp"

template<class T, template<typename> class VectorBase = Vector3DBase, class Line = VectorBase<T>, class Point = VectorBase<T>>
struct PrimitivePlaneBase{
public:
    bool isIn(const Point &pt){
        Line n1 = pt - p0;
        return n1.dot(n0) == 0;
    }
public:
    Point p0;           //平面上的点，通过一个点和法线来确定一个平面
    Line n0;             //平面的法线
};