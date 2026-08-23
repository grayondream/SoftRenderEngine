#pragma once
#include <cstdint>
#include <cfloat>
#include <cmath>
#include <vector>
#include <algorithm>

class FrameBuffer{
public:
    FrameBuffer(std::size_t width, std::size_t height)
        : m_width(width), m_height(height){
        m_color.resize(m_width * m_height, 0xFF000000u);
        m_depth.resize(m_width * m_height, FLT_MAX);
    }

    void setPixel(std::size_t x, std::size_t y, uint32_t bgra, float depth){
        if(x >= m_width || y >= m_height) return;
        if(!std::isfinite(depth)) return;
        auto idx = y * m_width + x;
        if(depth >= m_depth[idx]) return;
        m_depth[idx] = depth;
        m_color[idx] = bgra;
    }

    void clear(uint32_t bgra = 0xFF000000u){
        std::fill(m_color.begin(), m_color.end(), bgra);
        clearDepth();
    }

    void clearDepth(){
        std::fill(m_depth.begin(), m_depth.end(), FLT_MAX);
    }

    void blendPixel(std::size_t x, std::size_t y, uint32_t bgra, float depth){
        if(x >= m_width || y >= m_height) return;
        if(!std::isfinite(depth)) return;
        const uint32_t sa = (bgra >> 24) & 0xFF;
        if(sa == 0) return;
        auto idx = y * m_width + x;
        if(depth >= m_depth[idx]) return;
        if(sa == 255){
            m_depth[idx] = depth;
            m_color[idx] = bgra;
            return;
        }
        uint32_t dst = m_color[idx];
        auto mix = [sa](uint32_t s, uint32_t d){
            return static_cast<uint32_t>((s * sa + d * (255u - sa) + 127u) / 255u);
        };
        const uint32_t outR = mix((bgra >> 16) & 0xFF, (dst >> 16) & 0xFF);
        const uint32_t outG = mix((bgra >> 8) & 0xFF, (dst >> 8) & 0xFF);
        const uint32_t outB = mix(bgra & 0xFF, dst & 0xFF);
        m_depth[idx] = depth;
        m_color[idx] = 0xFF000000u | (outR << 16) | (outG << 8) | outB;
    }

    uint32_t* colorData(){ return m_color.data(); }
    const uint32_t* colorData() const{ return m_color.data(); }
    float* depthData(){ return m_depth.data(); }
    const float* depthData() const{ return m_depth.data(); }
    std::size_t width() const{ return m_width; }
    std::size_t height() const{ return m_height; }

private:
    std::size_t m_width{};
    std::size_t m_height{};
    std::vector<uint32_t> m_color{};
    std::vector<float> m_depth{};
};
