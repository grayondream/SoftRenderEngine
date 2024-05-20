#pragma once

#include <cstdint>

template<class T>
struct Vector2{
    T x;
    T y;
};

template<class T>
struct Vector3 : Vector2<T>{
    T z;
};

template<class T>
struct Vector4 : Vector3<T>{
    T w;
};

template<class T>
struct Position{
    Vector2<T> lf;
    Vector2<T> rb;

    T width() {
        return rb.x - lf.x;
    }

    T height(){
        return rb.y - lf.y;
    }
};


using PositionI32 = Position<int32_t>;
using PositionUi32 = Position<uint32_t>;
using PositionI64 = Position<int64_t>;
using PositionUi64 = Position<uint64_t>;
using PositionF = Position<float>;
using PositionD = Position<double>;

using ColorUi8 = Vector4<uint8_t>;

using WindowIDType = long int;

static constexpr int WINDOW_DEFAULT_X   = 100;
static constexpr int WINDOW_DEFAULT_Y  = 100;
static constexpr int WINDOW_DEFAULT_WIDTH   = 720;
static constexpr int WINDOW_DEFAULT_HEIGHT  = 480;
