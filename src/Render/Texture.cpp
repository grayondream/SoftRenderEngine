#include "Texture.hpp"
#include <cassert>
#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdint>
#include <vector>

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
    const double bot = static_cast<double>((c2 >> shift) & 0xFF) * (1.0 - ay)
                     + static_cast<double>((c3 >> shift) & 0xFF) * ay;
    return static_cast<uint8_t>(std::lround(top * (1.0 - ay) + bot * ay));
}
}

Texture::Texture(std::size_t w, std::size_t h, const uint32_t *bgra)
    : m_w(w), m_h(h){
    assert(bgra != nullptr || (w == 0 && h == 0));
    if(bgra){
        m_pixels.assign(bgra, bgra + w * h);
    }
    buildMipChain();
}

void Texture::buildMipChain(){
    m_mips.clear();
    if(m_pixels.empty()) return;
    m_mips.push_back(m_pixels);
    std::size_t lw = m_w, lh = m_h;
    while(lw > 1 || lh > 1){
        const std::size_t nw = std::max<std::size_t>(1, lw / 2);
        const std::size_t nh = std::max<std::size_t>(1, lh / 2);
        Level next(nw * nh);
        const Level &cur = m_mips.back();
        for(std::size_t y = 0; y < nh; y++){
            for(std::size_t x = 0; x < nw; x++){
                uint32_t acc[4] = {0, 0, 0, 0};
                int count = 0;
                for(std::size_t sy = 0; sy < 2; sy++){
                    for(std::size_t sx = 0; sx < 2; sx++){
                        const std::size_t cx = std::min(lw - 1, x*2 + sx);
                        const std::size_t cy = std::min(lh - 1, y*2 + sy);
                        const uint32_t c = cur[cy * lw + cx];
                        acc[0] += (c >> 24) & 0xFF;
                        acc[1] += (c >> 16) & 0xFF;
                        acc[2] += (c >> 8) & 0xFF;
                        acc[3] += c & 0xFF;
                        count++;
                    }
                }
                next[y * nw + x] =
                    (static_cast<uint32_t>(acc[0] / count) << 24) |
                    (static_cast<uint32_t>(acc[1] / count) << 16) |
                    (static_cast<uint32_t>(acc[2] / count) << 8)  |
                    static_cast<uint32_t>(acc[3] / count);
            }
        }
        m_mips.push_back(std::move(next));
        lw = nw;
        lh = nh;
    }
}

uint32_t Texture::fetchTexelFrom(const Level &lv, std::size_t lw, std::size_t lh,
                                 int tx, int ty, TextureWrap wrap) const{
    if(lv.empty() || lw == 0 || lh == 0) return 0xFF000000u;
    const int cx = WrapTexel(tx, static_cast<int>(lw) - 1, wrap);
    const int cy = WrapTexel(ty, static_cast<int>(lh) - 1, wrap);
    return lv[static_cast<std::size_t>(cy) * lw + static_cast<std::size_t>(cx)];
}

uint32_t Texture::sampleLevel(std::size_t level, double u, double v,
                              TextureFilter filter, TextureWrap wrap) const{
    if(level >= m_mips.size()) level = m_mips.size() - 1;
    if(m_mips.empty()) return 0xFF000000u;
    const Level &lv = m_mips[level];
    const std::size_t lw = (level == 0) ? m_w : m_mips[level].size() == 0 ? 0 : lv.size();
    // derive layer dims by successive halving from base
    std::size_t dimW = m_w, dimH = m_h;
    for(std::size_t i = 0; i < level; i++){
        dimW = std::max<std::size_t>(1, dimW / 2);
        dimH = std::max<std::size_t>(1, dimH / 2);
    }
    (void)lw;

    if(filter == TextureFilter::Nearest){
        const int tx = static_cast<int>(std::floor(u * static_cast<double>(dimW)));
        const int ty = static_cast<int>(std::floor(v * static_cast<double>(dimH)));
        return fetchTexelFrom(lv, dimW, dimH, tx, ty, wrap);
    }

    const double fx = u * static_cast<double>(dimW) - 0.5;
    const double fy = v * static_cast<double>(dimH) - 0.5;
    const int x0 = static_cast<int>(std::floor(fx));
    const int y0 = static_cast<int>(std::floor(fy));
    const double ax = fx - static_cast<double>(x0);
    const double ay = fy - static_cast<double>(y0);

    const uint32_t c00 = fetchTexelFrom(lv, dimW, dimH, x0,     y0,     wrap);
    const uint32_t c10 = fetchTexelFrom(lv, dimW, dimH, x0 + 1, y0,     wrap);
    const uint32_t c01 = fetchTexelFrom(lv, dimW, dimH, x0,     y0 + 1, wrap);
    const uint32_t c11 = fetchTexelFrom(lv, dimW, dimH, x0 + 1, y0 + 1, wrap);

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

uint32_t Texture::sampleTrilinear(double u, double v, float lod, TextureWrap wrap) const{
    if(m_mips.empty()) return sample(u, v, TextureFilter::Bilinear, wrap);
    const float clamped = std::clamp(lod, 0.0f,
        static_cast<float>(m_mips.size() - 1));
    const auto l0 = static_cast<std::size_t>(clamped);
    const auto l1 = std::min(m_mips.size() - 1, l0 + 1);
    const float frac = clamped - static_cast<float>(l0);
    const uint32_t a = sampleLevel(l0, u, v, TextureFilter::Bilinear, wrap);
    const uint32_t b = sampleLevel(l1, u, v, TextureFilter::Bilinear, wrap);
    auto mix = [frac](uint32_t sa, uint32_t sb){
        return static_cast<uint32_t>(
            static_cast<double>(sa) * (1.0f - frac) +
            static_cast<double>(sb) * frac + 0.5);
    };
    const uint32_t r = mix((a >> 16) & 0xFF, (b >> 16) & 0xFF);
    const uint32_t g = mix((a >> 8) & 0xFF, (b >> 8) & 0xFF);
    const uint32_t bl = mix(a & 0xFF, b & 0xFF);
    return 0xFF000000u | (r << 16) | (g << 8) | bl;
}

uint32_t Texture::sample(double u, double v,
                         TextureFilter filter, TextureWrap wrap) const{
    if(m_w == 0 || m_h == 0) return 0xFF000000u;
    if(!std::isfinite(u) || !std::isfinite(v)) return 0xFF000000u;

    const double maxU = static_cast<double>(INT_MAX) / static_cast<double>(m_w);
    const double maxV = static_cast<double>(INT_MAX) / static_cast<double>(m_h);
    u = std::clamp(u, -maxU, maxU);
    v = std::clamp(v, -maxV, maxV);

    return sampleLevel(0, u, v,
        filter == TextureFilter::Trilinear ? TextureFilter::Bilinear : filter, wrap);
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
