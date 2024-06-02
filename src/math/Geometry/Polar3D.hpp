#pragma once
#include "Vector3DBase.hpp"
#include <cmath>

template<class T>
class Vector3DBase;
//柱面坐标
template<class T>
class Polar3DBase{
    template<class U>
    Polar3DBase(const Vector3DBase<U> &pt){
        z = pt.z;
        r = std::sqrt(pt.x * pt.x + pt.y * pt.y);
        thetha = std::atan2(pt.y, pt.x);
    }
public:
    T r{};
    T thetha{};
    T z{};
};