#pragma once
#include <cstdint>

template<class T>
class Color{
public:
    union{
        T r, g, b, a;
        T color[4];
    };
};

using Color32 = Color<int32_t>;
using ColorFlt = Color<float>;