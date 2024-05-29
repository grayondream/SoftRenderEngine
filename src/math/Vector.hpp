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

using Vector2D = Vector2DBase<double>;
using Vector3D = Vector3DBase<double>;
using Vector4D = Vector4DBase<double>;

using Point2D = Vector2D;
using Point3D = Vector3D;
using Point4D = Vector4D;

using Vertex2D = Vector2D;
using Vertex3D = Vector3D;
using Vertex4D = Vector4D;
