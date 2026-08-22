#pragma once
#include "FrameBuffer.hpp"
#include "Texture.hpp"
#include "GeoObject/Color.hpp"

struct ScreenVertex{
    double x = 0;
    double y = 0;
    float z = 0;
    float w = 1;
    float u = 0;
    float v = 0;
    Color32 color{};
};

inline uint32_t PackBGRA(const Color32 &c){
    return (static_cast<uint32_t>(c.a) << 24) |
           (static_cast<uint32_t>(c.r) << 16) |
           (static_cast<uint32_t>(c.g) << 8)  |
            static_cast<uint32_t>(c.b);
}

class Rasterizer{
public:
    explicit Rasterizer(FrameBuffer &fb) : m_fb(fb){ }

    void drawLine(const ScreenVertex &a, const ScreenVertex &b);
    void drawTriangleWireframe(const ScreenVertex &v0, const ScreenVertex &v1, const ScreenVertex &v2);
    void drawTriangleSolid(const ScreenVertex &v0, const ScreenVertex &v1, const ScreenVertex &v2);
    void drawTriangleTextured(const ScreenVertex &v0, const ScreenVertex &v1,
                              const ScreenVertex &v2, const Texture &tex,
                              TextureFilter filter = TextureFilter::Bilinear,
                              TextureWrap wrap = TextureWrap::Repeat);

private:
    void plot(int x, int y, const Color32 &c);

    FrameBuffer &m_fb;
};
