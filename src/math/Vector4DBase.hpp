#pragma once

template<class T>
struct Vector4DBase{
    union{
        T data[4];
        struct{
            T x;
            T y;
            T z;
            T w;
        };
    };
};