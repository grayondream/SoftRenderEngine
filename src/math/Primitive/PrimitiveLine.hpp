#pragma once
#include "Vector/Vector.hpp"
#include "PrimitiveDefine.hpp"
#include "Vector2DBase.hpp"
#include <type_traits>
#include <utility>

template<class T, class Point = Vector2DBase<T>>
class PrimitiveLineBase{
public:
    std::pair<LineIntersect, Point> intersect(const PrimitiveLineBase& line);

    double k(){
        static_assert(std::is_same_v<Point, Vector2DBase<T>>, "do not support other vector");
        return static_cast<double>(p0.y - p1.y) / (p0.x - p1.x);
    }

    double b(){
        static_assert(std::is_same_v<Point, Vector2DBase<T>>, "do not support other vector");
        return p0.y - k() * p0.x;
    }

    std::pair<double, double> kb(){
        static_assert(std::is_same_v<Point, Vector2DBase<T>>, "do not support other vector");
        return {k(), b()};
    }

    double length(){
        return p0.distance(p1);
    }
public:
    Point p0;
    Point p1;
};


template<class T, class Point>
std::pair<LineIntersect, Point> PrimitiveLineBase<T, Point>::intersect(const PrimitiveLineBase& line){
    static_assert(std::is_same_v<Point, Vector2DBase<T>>, "do not support other vector");
    auto [k1, b1] = this->kb();
    auto [k2, b2] = line.kb();
    if(k1 == k2){
        return {LineIntersect::LineNoIntersect, {}};
    }

    auto x = b2 - b1 / (k1 - k2);
    auto y = k1 * x + b1;
    if( (x < std::max(p0.x, p1.x, line.p0.x, line.p1.x) && x > std::min(p0.x, p1.x, line.p0.x, line.p1.x)) 
        && (y < std::max(p0.y, p1.y, line.p0.y, line.p1.y) && y > std::max(p0.y, p1.y, line.p0.y, line.p1.y))){
        return {LineIntersect::LineIntersectInSegment, {x, y}};
    }

    return {LineIntersect::LineIntersectOutSegment, {x, y}};
}