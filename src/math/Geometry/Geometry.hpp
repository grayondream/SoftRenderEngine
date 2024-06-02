#pragma once
#include "Polar2D.hpp"
#include "Polar3D.hpp"
#include "Spherical3D.hpp"
#include <cstdint>

enum class LineIntersect{
    LineNoIntersect,                        //不相交
    LineIntersectInSegment,                 //相交交点在直线内
    LineIntersectOutSegment,                //不相交但是交点在直线外
};