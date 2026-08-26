#pragma once
#include <cstdint>

template<class T>
class Color{
public:
    Color() = default;
    Color(const T r, const T g, const T b, const T a = 1){
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    Color operator*(const Color &o) const{
        return Color{r * o.r, g * o.g, b * o.b, a * o.a};
    }

public:
    union{
        T color[4]{};
        struct{
            T r;
            T g;
            T b;
            T a;
        };
    };
};

using Color32 = Color<int32_t>;
using ColorFlt = Color<float>;