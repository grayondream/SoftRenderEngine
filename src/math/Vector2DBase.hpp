#pragma once

template<class T>
struct Vector2DBase{
    union{
        T data[2];
        struct{
            T x;
            T y;
        };
    };
};

namespace Utils{
namespace Vector{
    

}
}