#pragma once
#include "GeoObject/Color.hpp"
#include <cstdint>
#include <vector>
#include <math/Vector/Vector3DBase.hpp>

struct DirectionalLight{
    Vector3DBase<double> direction;
    ColorFlt color;
};

struct PointLight{
    Vector3DBase<double> position;
    ColorFlt color;
    double range{10.0};
};

struct LightingRig{
    float ambient{0.15f};
    float shininess{32.0f};
    float specularStrength{0.5f};
    std::vector<DirectionalLight> directional{};
    std::vector<PointLight> point{};
};

uint32_t shade(const LightingRig &rig,
               const Color32 &albedo,
               const Vector3DBase<double> &N,
               const Vector3DBase<double> &P,
               const Vector3DBase<double> &viewPos);
