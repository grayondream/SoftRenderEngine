#pragma once
#include "UUID.hpp"
#include "Vector.hpp"
#include "PolyF4D.hpp"

constexpr static const int kObject4vNameLen = 64;
constexpr static const int kObject4vListLen = 64;
constexpr static const int kPolyListLen = 128;


class Object4D{
public:
    enum class Object4VState{
        None,
        Active,
        Clipped,
        Backface
    };

    enum class Object4VAttr{
        None,
        SIDED2,
        TRANSPARENT,
        Color8Bit,
        RGB16,
        RGB24,
        ShadeModePure,
        ShadeModeFlat,
        ShadeModeGouraud,
        ShadeModePhong
    };
public:
    UUID id;
    char name[kObject4vNameLen]{};
    float avgRadius{};
    float maxRadius{};
    Point4D worldPos{};         //position in world
    Vector4D direction{};        //the direction in local point
    Vector4D ux;
    Vector4D uy;
    Vector4D uz;
    int numVertices{};          //number ofr vertices
    Point4D vlistLocal[kObject4vListLen]{};
    Point4D vlistWorld[kObject4vListLen]{};
    int numPolys{};
    PolyF4D plist[kPolyListLen];
};