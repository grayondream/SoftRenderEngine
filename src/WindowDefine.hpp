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

struct Size{
    std::size_t width{};
    std::size_t height{};
};

template<class T>
struct Position2d{
    Vector2<T> lf;
    Size size;
};


using Position = Position2d<int32_t>;
using Color = Vector4<uint8_t>;
using Point2D = Vector2<int32_t>;

using WindowIDType = long int;

static constexpr int WINDOW_DEFAULT_X   = 100;
static constexpr int WINDOW_DEFAULT_Y  = 100;
static constexpr int WINDOW_DEFAULT_WIDTH   = 720;
static constexpr int WINDOW_DEFAULT_HEIGHT  = 480;
