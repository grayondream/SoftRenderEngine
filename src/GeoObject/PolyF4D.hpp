#pragma once
#include "Vector.hpp"
#include "Color.hpp"

constexpr const static int32_t kTriangleVerticesNumber = 3;

class PolyF4D{
public:
    enum class PolyState : int32_t {
        None
    };

    enum class PolyAttr : int32_t {
        None
    };
public:
    PolyState state;
    PolyAttr attr;
    Color32 color;
    Point4D vlist[kTriangleVerticesNumber];
    Point4D tvlist[kTriangleVerticesNumber];
    PolyF4D *next{};
    PolyF4D *prev{};
};