#pragma once
#include <cstdint>
#include <cfloat>
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

    uint32_t* colorData(){ return m_color.data(); }
    const uint32_t* colorData() const{ return m_color.data(); }
    float* depthData(){ return m_depth.data(); }
    std::size_t width() const{ return m_width; }
    std::size_t height() const{ return m_height; }

private:
    std::size_t m_width{};
    std::size_t m_height{};
    std::vector<uint32_t> m_color{};
    std::vector<float> m_depth{};
};
