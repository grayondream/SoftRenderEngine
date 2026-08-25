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

struct SpotLight{
    Vector3DBase<double> position;
    Vector3DBase<double> direction;
    ColorFlt color;
    double range{12.0};
    double cutoffCos{0.85};
};

struct LightingRig{
    float ambient{0.15f};
    ColorFlt ambientColor{1.0f, 1.0f, 1.0f, 1.0f};
    float shininess{32.0f};
    float specularStrength{0.5f};
    std::vector<DirectionalLight> directional{};
    std::vector<PointLight> point{};
    std::vector<SpotLight> spot{};
};

class Texture;

struct PbrMaterial{
    Color32 baseColor{200, 60, 60, 255};
    float metallic{0.0f};
    float roughness{0.35f};
    const class Texture *albedoTex{};
    const class Texture *metallicTex{};
    const class Texture *roughnessTex{};
};

uint32_t pbrShade(const LightingRig &rig,
                  const PbrMaterial &mat,
                  const Vector3DBase<double> &N,
                  const Vector3DBase<double> &P,
                  const Vector3DBase<double> &viewPos,
                  double shadowFactor = 1.0);

uint32_t shade(const LightingRig &rig,
               const Color32 &albedo,
               const Vector3DBase<double> &N,
               const Vector3DBase<double> &P,
               const Vector3DBase<double> &viewPos,
               double shadowFactor = 1.0);
