#pragma once

//球面坐标
#include "Vector3DBase.hpp"
#include <cmath>

template<class T>
class Vector3DBase;

template<class T>
class Spherical3DBase{
public:
    Spherical3DBase() = default;
    Spherical3DBase(const T ri, const T t, const T p){
        r = ri;
        thetha = t;
        phi = p;
    }

    template<class U>
    Spherical3DBase(const Vector3DBase<U> &pt){
        r = std::sqrt(pt.x * pt.x + pt.y * pt.y + pt.z * pt.z);
        thetha = std::acos(pt.z / r);
        phi = std::atan2(pt.y, pt.x);
    }
public:
    T r{};
    T thetha{};                 //r 与 z轴的夹角
    T phi{};                    //xy上点与x轴夹角
};