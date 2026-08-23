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

void Rasterizer::drawTriangleDepth(const ScreenVertex &v0, const ScreenVertex &v1,
                                   const ScreenVertex &v2){
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

    bool tl0 = IsTopLeftEdge(v1.x,v1.y, v2.x,v2.y);
    bool tl1 = IsTopLeftEdge(v2.x,v2.y, v0.x,v0.y);
    bool tl2 = IsTopLeftEdge(v0.x,v0.y, v1.x,v1.y);

    constexpr double eps = 1e-9;
    float *depth = m_fb.depthData();

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

            double iw = w0/v0.w + w1/v1.w + w2/v2.w;
            if(iw <= 0) continue;
            const float zNdc = static_cast<float>(
                (w0*v0.z/v0.w + w1*v1.z/v1.w + w2*v2.z/v2.w) / iw);
            auto idx = static_cast<std::size_t>(y)*m_fb.width() + static_cast<std::size_t>(x);
            if(zNdc < depth[idx]){
                depth[idx] = zNdc;
            }
        }
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

            m_fb.blendPixel(static_cast<std::size_t>(x), static_cast<std::size_t>(y),
                            packed, zNdc);
        }
    }
}

void Rasterizer::drawTriangleTextured(const ScreenVertex &v0, const ScreenVertex &v1,
                                      const ScreenVertex &v2, const Texture &tex,
                                      const ShadingContext *shading,
                                      TextureFilter filter, TextureWrap wrap,
                                      const ScreenRect *clip){
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
    if(clip){
        x0 = std::max(x0, clip->x0);
        y0 = std::max(y0, clip->y0);
        x1 = std::min(x1, clip->x1);
        y1 = std::min(y1, clip->y1);
    }
    if(x0 > x1 || y0 > y1) return;

    bool tl0 = IsTopLeftEdge(v1.x,v1.y, v2.x,v2.y);
    bool tl1 = IsTopLeftEdge(v2.x,v2.y, v0.x,v0.y);
    bool tl2 = IsTopLeftEdge(v0.x,v0.y, v1.x,v1.y);

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

            double iw = w0/v0.w + w1/v1.w + w2/v2.w;
            if(iw <= 0) continue;
            float zNdc = static_cast<float>(
                (w0*v0.z/v0.w + w1*v1.z/v1.w + w2*v2.z/v2.w) / iw);
            float uPix = static_cast<float>(
                (w0*v0.u/v0.w + w1*v1.u/v1.w + w2*v2.u/v2.w) / iw);
            float vPix = static_cast<float>(
                (w0*v0.v/v0.w + w1*v1.v/v1.w + w2*v2.v/v2.w) / iw);

            double nxc = 0, nyc = 0, nzc = 0;
            double wxp = 0, wyp = 0, wzp = 0;
            if(shading && (shading->rig || shading->fog)){
                nxc = (w0*v0.nx/v0.w + w1*v1.nx/v1.w + w2*v2.nx/v2.w) / iw;
                nyc = (w0*v0.ny/v0.w + w1*v1.ny/v1.w + w2*v2.ny/v2.w) / iw;
                nzc = (w0*v0.nz/v0.w + w1*v1.nz/v1.w + w2*v2.nz/v2.w) / iw;
                wxp = (w0*v0.wx/v0.w + w1*v1.wx/v1.w + w2*v2.wx/v2.w) / iw;
                wyp = (w0*v0.wy/v0.w + w1*v1.wy/v1.w + w2*v2.wy/v2.w) / iw;
                wzp = (w0*v0.wz/v0.w + w1*v1.wz/v1.w + w2*v2.wz/v2.w) / iw;
            }

            uint32_t shaded = tex.sample(uPix, vPix, filter, wrap);
            double shadowFactor = 1.0;
            if(shading && shading->shadow && shading->shadow->depth){
                const auto &sd = *shading->shadow;
                const Vector4DBase<double> Pl{wxp, wyp, wzp, 1.0};
                const Matrix4DBase<double> &M = sd.lightViewProj;
                double lx = M[0][0][0][0]*Pl.x + M[0][0][0][1]*Pl.y + M[0][0][0][2]*Pl.z + M[0][0][0][3]*Pl.w;
                double ly = M[0][0][1][0]*Pl.x + M[0][0][1][1]*Pl.y + M[0][0][1][2]*Pl.z + M[0][0][1][3]*Pl.w;
                double lz = M[0][0][2][0]*Pl.x + M[0][0][2][1]*Pl.y + M[0][0][2][2]*Pl.z + M[0][0][2][3]*Pl.w;
                double lw = M[0][0][3][0]*Pl.x + M[0][0][3][1]*Pl.y + M[0][0][3][2]*Pl.z + M[0][0][3][3]*Pl.w;
                if(lw > 1e-9){
                    const double nx = lx / lw;
                    const double ny = ly / lw;
                    const double nz = lz / lw;
                    if(nx >= -1.0 && nx <= 1.0 && ny >= -1.0 && ny <= 1.0){
                        const std::size_t smW = sd.depth->width();
                        const std::size_t smH = sd.depth->height();
                        auto sx = static_cast<std::size_t>((nx * 0.5 + 0.5) * static_cast<double>(smW - 1));
                        auto sy = static_cast<std::size_t>((ny * -0.5 + 0.5) * static_cast<double>(smH - 1));
                        const float zMain01 = static_cast<float>(nz * 0.5 + 0.5);
                        if(sd.pcfRadius <= 0){
                            const float dLight = sd.depth->depthData()[sy * smW + sx];
                            if(zMain01 > dLight + static_cast<float>(sd.bias)){
                                shadowFactor = 0.0;
                            }
                        }else{
                            int occluded = 0, total = 0;
                            for(int oy = -sd.pcfRadius; oy <= sd.pcfRadius; oy++){
                                for(int ox = -sd.pcfRadius; ox <= sd.pcfRadius; ox++){
                                    const auto px2 = std::clamp<std::int64_t>(
                                        static_cast<std::int64_t>(sx) + ox, 0,
                                        static_cast<std::int64_t>(smW - 1));
                                    const auto py2 = std::clamp<std::int64_t>(
                                        static_cast<std::int64_t>(sy) + oy, 0,
                                        static_cast<std::int64_t>(smH - 1));
                                    const float dLight = sd.depth->depthData()[
                                        static_cast<std::size_t>(py2) * smW + static_cast<std::size_t>(px2)];
                                    if(zMain01 > dLight + static_cast<float>(sd.bias)){
                                        occluded++;
                                    }
                                    total++;
                                }
                            }
                            shadowFactor = 1.0 - static_cast<double>(occluded) / total;
                        }
                    }
                }
            }else if(shading && shading->cubeShadow){
                const auto &cs = *shading->cubeShadow;
                const Vector3DBase<double> L{wxp - cs.lightPos.x,
                                             wyp - cs.lightPos.y,
                                             wzp - cs.lightPos.z};
                const int face = SGE::Render::cubeFaceIndex(L);
                const FrameBuffer *faceFb = cs.faces[face];
                if(faceFb){
                    const auto M = SGE::Render::cubeFaceVP(cs.lightPos, face);
                    double lx = M[0][0][0][0]*wxp + M[0][0][0][1]*wyp + M[0][0][0][2]*wzp + M[0][0][0][3];
                    double ly = M[0][0][1][0]*wxp + M[0][0][1][1]*wyp + M[0][0][1][2]*wzp + M[0][0][1][3];
                    double lz = M[0][0][2][0]*wxp + M[0][0][2][1]*wyp + M[0][0][2][2]*wzp + M[0][0][2][3];
                    double lw = M[0][0][3][0]*wxp + M[0][0][3][1]*wyp + M[0][0][3][2]*wzp + M[0][0][3][3];
                    if(lw > 1e-9){
                        const double nx = lx / lw;
                        const double ny = ly / lw;
                        const double nz = lz / lw;
                        if(nx >= -1.0 && nx <= 1.0 && ny >= -1.0 && ny <= 1.0){
                            const std::size_t smW = faceFb->width();
                            const std::size_t smH = faceFb->height();
                            auto sx = static_cast<std::size_t>((nx * 0.5 + 0.5) * static_cast<double>(smW - 1));
                            auto sy = static_cast<std::size_t>((ny * -0.5 + 0.5) * static_cast<double>(smH - 1));
                            const float zMain01 = static_cast<float>(nz * 0.5 + 0.5);
                            if(cs.pcfRadius <= 0){
                                const float dLight = faceFb->depthData()[sy * smW + sx];
                                if(zMain01 > dLight + static_cast<float>(cs.bias)){
                                    shadowFactor = 0.0;
                                }
                            }else{
                                int occluded = 0, total = 0;
                                for(int oy = -cs.pcfRadius; oy <= cs.pcfRadius; oy++){
                                    for(int ox = -cs.pcfRadius; ox <= cs.pcfRadius; ox++){
                                        const auto px2 = std::clamp<std::int64_t>(
                                            static_cast<std::int64_t>(sx) + ox, 0,
                                            static_cast<std::int64_t>(smW - 1));
                                        const auto py2 = std::clamp<std::int64_t>(
                                            static_cast<std::int64_t>(sy) + oy, 0,
                                            static_cast<std::int64_t>(smH - 1));
                                        const float dLight = faceFb->depthData()[
                                            static_cast<std::size_t>(py2) * smW + static_cast<std::size_t>(px2)];
                                        if(zMain01 > dLight + static_cast<float>(cs.bias)){
                                            occluded++;
                                        }
                                        total++;
                                    }
                                }
                                shadowFactor = 1.0 - static_cast<double>(occluded) / total;
                            }
                        }
                    }
                }
            }
            if(shading && shading->rig){
                const Color32 albedo{
                    static_cast<int32_t>((shaded >> 16) & 0xFF),
                    static_cast<int32_t>((shaded >> 8) & 0xFF),
                    static_cast<int32_t>(shaded & 0xFF), 255};
                const Vector3DBase<double> N{nxc, nyc, nzc};
                const Vector3DBase<double> Pw{wxp, wyp, wzp};
                shaded = shade(*shading->rig, albedo, N.normalize(), Pw, shading->viewPos, shadowFactor);
            }
            if(shading && shading->fog){
                const FogParams &fog = *shading->fog;
                const double dx = wxp - shading->viewPos.x;
                const double dy = wyp - shading->viewPos.y;
                const double dz = wzp - shading->viewPos.z;
                const double dist = std::sqrt(dx*dx + dy*dy + dz*dz);
                double f = (dist - fog.start) / (fog.end - fog.start);
                if(!(f >= 0)) f = 0;
                if(f > 1) f = 1;
                if(f > 0){
                    auto mixCh = [f](uint32_t shadedC, float fogC) -> uint32_t {
                        const double v = shadedC + (fogC * 255.0 - shadedC) * f;
                        return static_cast<uint32_t>(v + 0.5);
                    };
                    const uint32_t outR = mixCh((shaded >> 16) & 0xFF, fog.color.r);
                    const uint32_t outG = mixCh((shaded >> 8) & 0xFF, fog.color.g);
                    const uint32_t outB = mixCh(shaded & 0xFF, fog.color.b);
                    shaded = 0xFF000000u | (outR << 16) | (outG << 8) | outB;
                }
            }
            m_fb.blendPixel(static_cast<std::size_t>(x), static_cast<std::size_t>(y),
                            shaded, zNdc);
        }
    }
}
