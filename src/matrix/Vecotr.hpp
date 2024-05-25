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

template<class T>
struct Vector3D{
    union{
        T data[3];
        struct{
            T x;
            T y;
            T z;
        };
    };
};

template<class T>
struct Vector4D{
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