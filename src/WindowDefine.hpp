#pragma once

#include <cstdint>


struct Point2D{
    std::size_t x;
    std::size_t y;
};

struct Size{
    std::size_t width{};
    std::size_t height{};
};

struct Position2d{
    Point2D lf;
    Size size;
};

template<class T>
struct ColorBase{
    T r;
    T g;
    T b;
    T a;
};

enum class RenderFormat : int32_t{
    RGBA8888,
    BGRA8888,
};

using Position = Position2d;
using Color = ColorBase<uint8_t>;

using WindowIDType = long int;

static constexpr int WINDOW_DEFAULT_X   = 100;
static constexpr int WINDOW_DEFAULT_Y  = 100;
static constexpr int WINDOW_DEFAULT_WIDTH   = 720;
static constexpr int WINDOW_DEFAULT_HEIGHT  = 480;
