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

    double k() const{
        static_assert(std::is_same_v<Point, Vector2DBase<T>>, "do not support other vector");
        return static_cast<double>(p0.y - p1.y) / (p0.x - p1.x);
    }

    double b() const{
        static_assert(std::is_same_v<Point, Vector2DBase<T>>, "do not support other vector");
        return p0.y - k() * p0.x;
    }

    std::pair<double, double> kb() const{
        static_assert(std::is_same_v<Point, Vector2DBase<T>>, "do not support other vector");
        return {k(), b()};
    }

    double length() const{
        return p0.distance(p1);
    }
public:
    Point p0;
    Point p1;
};


template<class T, class Point>
std::pair<LineIntersect, Point> PrimitiveLineBase<T, Point>::intersect(const PrimitiveLineBase& line){
    static_assert(std::is_same_v<Point, Vector2DBase<T>>, "do not support other vector");
    const auto [k1, b1] = this->kb();
    const auto [k2, b2] = line.kb();
    if(k1 == k2){
        return {LineIntersect::LineNoIntersect, {}};
    }

    const auto x = (b2 - b1) / (k1 - k2);
    const auto y = k1 * x + b1;

    const auto minX = std::min(std::min(p0.x, p1.x), std::min(line.p0.x, line.p1.x));
    const auto maxX = std::max(std::max(p0.x, p1.x), std::max(line.p0.x, line.p1.x));
    const auto minY = std::min(std::min(p0.y, p1.y), std::min(line.p0.y, line.p1.y));
    const auto maxY = std::max(std::max(p0.y, p1.y), std::max(line.p0.y, line.p1.y));

    if(x > minX && x < maxX && y > minY && y < maxY){
        return {LineIntersect::LineIntersectInSegment, {x, y}};
    }

    return {LineIntersect::LineIntersectOutSegment, {x, y}};
}