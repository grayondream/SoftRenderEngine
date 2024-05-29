#pragma once
#include "Vector.hpp"

template<class T, class Point = Vector2DBase<T>>
class PrimitiveLineBase{
public:
    Point p0;
    Point p1;
};

namespace Utils{
namespace Line{
    
}
}