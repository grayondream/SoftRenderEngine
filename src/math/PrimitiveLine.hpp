#pragma once
#include "Vector.hpp"

template<class T, class Point = Vector2DBase<T>>
struct PrimitiveLineBase{
    Point p0;
    Point p1;
};

namespace Utils{
namespace Line{
    
}
}