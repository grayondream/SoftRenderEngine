#pragma once

template<class T>
struct Vector3DBase{
    union{
        T data[3];
        struct{
            T x;
            T y;
            T z;
        };
    };
};
