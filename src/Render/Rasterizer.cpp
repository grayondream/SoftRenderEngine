#include "Rasterizer.hpp"
#include <algorithm>
#include <cmath>

void Rasterizer::plot(int x, int y, const Color32 &c){
    if(x < 0 || y < 0 || x >= static_cast<int>(m_fb.width()) || y >= static_cast<int>(m_fb.height())) return;
    m_fb.colorData()[static_cast<std::size_t>(y)*m_fb.width() + static_cast<std::size_t>(x)] = PackBGRA(c);
}

void Rasterizer::drawLine(const ScreenVertex &a, const ScreenVertex &b){
    int x0 = static_cast<int>(std::lround(a.x)), y0 = static_cast<int>(std::lround(a.y));
    int x1 = static_cast<int>(std::lround(b.x)), y1 = static_cast<int>(std::lround(b.y));
    int dx = std::abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = -std::abs(y1-y0), sy = y0<y1 ? 1 : -1;
    int err = dx + dy;
    while(true){
        plot(x0, y0, a.color);
        if(x0 == x1 && y0 == y1) break;
        auto e2 = 2*err;
        if(e2 >= dy){ err += dy; x0 += sx; }
        if(e2 <= dx){ err += dx; y0 += sy; }
    }
}

void Rasterizer::drawTriangleWireframe(const ScreenVertex &v0, const ScreenVertex &v1, const ScreenVertex &v2){
    drawLine(v0, v1);
    drawLine(v1, v2);
    drawLine(v2, v0);
}
