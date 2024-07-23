#pragma once
#include <cstdint>
#include "GeoObject/PolyF4D.hpp"

enum class RenderListState : int32_t{
    None,
};

enum class RenderListAttr : int32_t{
    None,
};

class RenderList4D{
public:
    static constexpr const int32_t kMaxPolys = 128;

public:
    void reset();

private:
    RenderListState _state;
    RenderListAttr _attr;
    PolyF4D *_polys[kMaxPolys]{};
    PolyF4D *_polyData[kMaxPolys]{};
    int _numberOfPolys{};
};