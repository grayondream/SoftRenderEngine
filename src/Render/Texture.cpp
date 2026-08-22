#include "Texture.hpp"
#include <algorithm>
#include <cmath>

namespace{
int WrapTexel(int t, int maxIdx, TextureWrap wrap){
    if(wrap == TextureWrap::Clamp){
        return std::clamp(t, 0, maxIdx);
    }
    const int size = maxIdx + 1;
    return ((t % size) + size) % size;
}
}

Texture::Texture(std::size_t w, std::size_t h, const uint32_t *bgra)
    : m_w(w), m_h(h){
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
                         TextureFilter /*filter*/, TextureWrap wrap) const{
    if(m_w == 0 || m_h == 0) return 0xFF000000u;

    const int tx = static_cast<int>(std::floor(u * static_cast<double>(m_w)));
    const int ty = static_cast<int>(std::floor(v * static_cast<double>(m_h)));
    return fetchTexel(tx, ty, wrap);
}
