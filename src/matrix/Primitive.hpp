#pragma once
#include "Vector.hpp"


template<class T, class Point = Vector2DBase<T>>
struct PrimitiveLineBase{
    Point p0;
    Point p1;
};

template<class T, template<typename> class VectorBase = Vector3DBase, class Line = VectorBase<T>, class Point = VectorBase<T>>
struct PrimitivePlaneBase{
    Point p0;           //平面上的点，通过一个点和法线来确定一个平面
    Line n;             //平面的法线
};

using Line2D = PrimitiveLineBase<double, Vector2DBase<double>>;
using Line3D = PrimitiveLineBase<double, Vector3DBase<double>>;
using Line4D = PrimitiveLineBase<double, Vector4DBase<double>>;

using Plane3D = PrimitivePlaneBase<double>;
using Plane4D = PrimitivePlaneBase<double, Vector4DBase>;

