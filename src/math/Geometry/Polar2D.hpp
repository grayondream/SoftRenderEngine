#pragma once
#include "Vector2DBase.hpp"
#include <cmath>
template<class T>
class Vector2DBase;
//极坐标
template<class T>
class Polar2DBase{
public:
    Polar2DBase() = default;
    Polar2DBase(const T ri, const T t){
        r = ri;
        thetha = t;
    }
    
    template<class U>
    Polar2DBase(const Vector2DBase<U> &pt){
        r = std::sqrt(pt.x * pt.x + pt.y * pt.y);
        thetha = atan2(pt.y, pt.x);
    }
public:
    T r{};
    T thetha{};         //单位为弧度
};