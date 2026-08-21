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

namespace{
double EdgeFunction(double ax, double ay, double bx, double by, double px, double py){
    return (bx-ax)*(py-ay) - (by-ay)*(px-ax);
}

// y-down screen space, area>0 winding:
// top edge    = horizontal, directed left-to-right
// left edge   = directed downward
bool IsTopLeftEdge(double ax, double ay, double bx, double by){
    if(ay == by) return ax < bx;
    return by > ay;
}
}

void Rasterizer::drawTriangleSolid(const ScreenVertex &v0, const ScreenVertex &v1, const ScreenVertex &v2){
    double area = EdgeFunction(v0.x,v0.y, v1.x,v1.y, v2.x,v2.y);
    if(area == 0) return;
    double invArea = 1.0 / area;

    double minX = std::min({v0.x, v1.x, v2.x});
    double maxX = std::max({v0.x, v1.x, v2.x});
    double minY = std::min({v0.y, v1.y, v2.y});
    double maxY = std::max({v0.y, v1.y, v2.y});
    int x0 = std::max(0, static_cast<int>(std::floor(minX)));
    int y0 = std::max(0, static_cast<int>(std::floor(minY)));
    int x1 = std::min(static_cast<int>(m_fb.width()) - 1,  static_cast<int>(std::ceil(maxX)));
    int y1 = std::min(static_cast<int>(m_fb.height()) - 1, static_cast<int>(std::ceil(maxY)));

    // top-left status per directed edge: w0->(v1,v2), w1->(v2,v0), w2->(v0,v1)
    bool tl0 = IsTopLeftEdge(v1.x,v1.y, v2.x,v2.y);
    bool tl1 = IsTopLeftEdge(v2.x,v2.y, v0.x,v0.y);
    bool tl2 = IsTopLeftEdge(v0.x,v0.y, v1.x,v1.y);

    uint32_t packed = PackBGRA(v0.color);
    constexpr double eps = 1e-9;

    for(int y = y0; y <= y1; y++){
        for(int x = x0; x <= x1; x++){
            double px = x + 0.5, py = y + 0.5;
            double w0 = EdgeFunction(v1.x,v1.y, v2.x,v2.y, px,py) * invArea;
            double w1 = EdgeFunction(v2.x,v2.y, v0.x,v0.y, px,py) * invArea;
            double w2 = EdgeFunction(v0.x,v0.y, v1.x,v1.y, px,py) * invArea;

            auto inside = [&](double w, bool topLeft){
                return w > eps || (topLeft && w >= -eps);
            };
            if(!inside(w0, tl0) || !inside(w1, tl1) || !inside(w2, tl2)) continue;

            // perspective-correct depth: interpolate z/w over interpolated 1/w
            double iw = w0/v0.w + w1/v1.w + w2/v2.w;
            if(iw <= 0) continue;
            float zNdc = static_cast<float>(
                (w0*v0.z/v0.w + w1*v1.z/v1.w + w2*v2.z/v2.w) / iw);

            m_fb.setPixel(static_cast<std::size_t>(x), static_cast<std::size_t>(y),
                          packed, zNdc);
        }
    }
}
