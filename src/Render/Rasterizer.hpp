#pragma once
#include "FrameBuffer.hpp"
#include "Texture.hpp"
#include "Light.hpp"
#include "Shadow.hpp"
#include "GeoObject/Color.hpp"

struct ScreenVertex{
    double x = 0;
    double y = 0;
    float z = 0;
    float w = 1;
    float u = 0;
    float v = 0;
    double nx = 0;
    double ny = 0;
    double nz = 0;
    double wx = 0;
    double wy = 0;
    double wz = 0;
    Color32 color{255, 255, 255, 255};
};

inline uint32_t PackBGRA(const Color32 &c){
    return (static_cast<uint32_t>(c.a) << 24) |
           (static_cast<uint32_t>(c.r) << 16) |
           (static_cast<uint32_t>(c.g) << 8)  |
            static_cast<uint32_t>(c.b);
}

using SGE::Render::ShadowData;
using SGE::Render::CubeShadowData;

struct FogParams{
    double start{};
    double end{};
    ColorFlt color{0.5f, 0.5f, 0.5f};
};

struct ShadingContext{
    const LightingRig *rig{};
    Vector3DBase<double> viewPos{};
    const FogParams *fog{};
    const ShadowData *shadow{};
    const CubeShadowData *cubeShadow{};
};

struct ScreenRect{
    int x0{};
    int y0{};
    int x1{};
    int y1{};
};

class Rasterizer{
public:
    explicit Rasterizer(FrameBuffer &fb) : m_fb(fb){ }

    void drawLine(const ScreenVertex &a, const ScreenVertex &b);
    void drawTriangleWireframe(const ScreenVertex &v0, const ScreenVertex &v1, const ScreenVertex &v2);
    void drawTriangleDepth(const ScreenVertex &v0, const ScreenVertex &v1, const ScreenVertex &v2);
    void drawTriangleSolid(const ScreenVertex &v0, const ScreenVertex &v1, const ScreenVertex &v2);
    void drawTriangleTextured(const ScreenVertex &v0, const ScreenVertex &v1,
                              const ScreenVertex &v2, const Texture &tex,
                              const ShadingContext *shading = nullptr,
                              TextureFilter filter = TextureFilter::Bilinear,
                              TextureWrap wrap = TextureWrap::Repeat,
                              const ScreenRect *clip = nullptr);

private:
    void plot(int x, int y, const Color32 &c);

    FrameBuffer &m_fb;
};
