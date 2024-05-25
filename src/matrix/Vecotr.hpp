#pragma once

template<class T>
struct Vector2D{
    union{
        T data[2];
        struct{
            T x;
            T y;
        };
    };
};
