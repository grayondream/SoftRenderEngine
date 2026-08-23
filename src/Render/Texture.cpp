#include "Texture.hpp"
#include <cassert>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <climits>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace{
int WrapTexel(int t, int maxIdx, TextureWrap wrap){
    if(wrap == TextureWrap::Clamp){
        return std::clamp(t, 0, maxIdx);
    }
    const int size = maxIdx + 1;
    return ((t % size) + size) % size;
}

uint8_t BlendChannel(uint32_t c0, uint32_t c1, uint32_t c2, uint32_t c3,
                     double ax, double ay, int shift){
    const double top = static_cast<double>((c0 >> shift) & 0xFF) * (1.0 - ax)
                     + static_cast<double>((c1 >> shift) & 0xFF) * ax;
    const double bot = static_cast<double>((c2 >> shift) & 0xFF) * (1.0 - ax)
                     + static_cast<double>((c3 >> shift) & 0xFF) * ax;
    return static_cast<uint8_t>(std::lround(top * (1.0 - ay) + bot * ay));
}
}

Texture::Texture(std::size_t w, std::size_t h, const uint32_t *bgra)
    : m_w(w), m_h(h){
    assert(bgra != nullptr || (w == 0 && h == 0));
    if(bgra){
        m_pixels.assign(bgra, bgra + w * h);
    }
}

uint32_t Texture::fetchTexel(int tx, int ty, TextureWrap wrap) const{
    if(m_w == 0 || m_h == 0) return 0xFF000000u;
    const int cx = WrapTexel(tx, static_cast<int>(m_w) - 1, wrap);
    const int cy = WrapTexel(ty, static_cast<int>(m_h) - 1, wrap);
    return m_pixels[static_cast<std::size_t>(cy) * m_w + static_cast<std::size_t>(cx)];
}

uint32_t Texture::sample(double u, double v,
                         TextureFilter filter, TextureWrap wrap) const{
    if(m_w == 0 || m_h == 0) return 0xFF000000u;
    if(!std::isfinite(u) || !std::isfinite(v)) return 0xFF000000u;

    const double maxU = static_cast<double>(INT_MAX) / static_cast<double>(m_w);
    const double maxV = static_cast<double>(INT_MAX) / static_cast<double>(m_h);
    u = std::clamp(u, -maxU, maxU);
    v = std::clamp(v, -maxV, maxV);

    if(filter == TextureFilter::Nearest){
        const int tx = static_cast<int>(std::floor(u * static_cast<double>(m_w)));
        const int ty = static_cast<int>(std::floor(v * static_cast<double>(m_h)));
        return fetchTexel(tx, ty, wrap);
    }

    const double fx = u * static_cast<double>(m_w) - 0.5;
    const double fy = v * static_cast<double>(m_h) - 0.5;
    const int x0 = static_cast<int>(std::floor(fx));
    const int y0 = static_cast<int>(std::floor(fy));
    const double ax = fx - static_cast<double>(x0);
    const double ay = fy - static_cast<double>(y0);

    const uint32_t c00 = fetchTexel(x0,     y0,     wrap);
    const uint32_t c10 = fetchTexel(x0 + 1, y0,     wrap);
    const uint32_t c01 = fetchTexel(x0,     y0 + 1, wrap);
    const uint32_t c11 = fetchTexel(x0 + 1, y0 + 1, wrap);

    auto pack = [](uint8_t a, uint8_t r, uint8_t g, uint8_t b) -> uint32_t {
        return (static_cast<uint32_t>(a) << 24) |
               (static_cast<uint32_t>(r) << 16) |
               (static_cast<uint32_t>(g) << 8)  |
                static_cast<uint32_t>(b);
    };
    return pack(BlendChannel(c00,c10,c01,c11,ax,ay,24),
                BlendChannel(c00,c10,c01,c11,ax,ay,16),
                BlendChannel(c00,c10,c01,c11,ax,ay,8),
                BlendChannel(c00,c10,c01,c11,ax,ay,0));
}

Texture Texture::loadFromFile(const std::string &path){
    int w = 0, h = 0, n = 0;
    stbi_uc *data = stbi_load(path.c_str(), &w, &h, &n, 4);
    if(!data || w <= 0 || h <= 0){
        if(data) stbi_image_free(data);
        return Texture{};
    }

    std::vector<uint32_t> pixels(static_cast<std::size_t>(w) * h);
    for(std::size_t i = 0; i < pixels.size(); i++){
        pixels[i] = (static_cast<uint32_t>(data[i*4+3]) << 24) |
                    (static_cast<uint32_t>(data[i*4+0]) << 16) |
                    (static_cast<uint32_t>(data[i*4+1]) << 8)  |
                     static_cast<uint32_t>(data[i*4+2]);
    }
    stbi_image_free(data);
    return Texture(static_cast<std::size_t>(w), static_cast<std::size_t>(h), pixels.data());
}
