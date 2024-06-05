#pragma once
#include "Vector/Vector.hpp"

template<class T, class Point = Vector2DBase<T>>
class PrimitiveLineBase{
public:
    Point p0;
    Point p1;
};
