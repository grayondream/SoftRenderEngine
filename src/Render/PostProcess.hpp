#pragma once

#include "FrameBuffer.hpp"
#include "ImageLoader.hpp"
#include "Camera.hpp"

#include <cmath>
#include <functional>
#include <vector>

namespace SGE::Render {

using PixelFn = std::function<Color32(double u, double v, const FrameBuffer &src)>;

inline void RunPass(const FrameBuffer &src, FrameBuffer &dst, const PixelFn &fn){
    const std::size_t w = dst.width(), h = dst.height();
    for(std::size_t y = 0; y < h; y++){
        const double v = (static_cast<double>(y) + 0.5) / h;
        for(std::size_t x = 0; x < w; x++){
            const double u = (static_cast<double>(x) + 0.5) / w;
            const Color32 rc = fn(u, v, src);
            dst.setPixel(x, y,
                (static_cast<uint32_t>(rc.a) << 24)
                    | (static_cast<uint32_t>(rc.r) << 16)
                    | (static_cast<uint32_t>(rc.g) << 8)
                    | static_cast<uint32_t>(rc.b), -2.0f);
        }
    }
}

struct Color3f{
    double r{}, g{}, b{};
};

inline Color3f fetchRGB(const FrameBuffer &src, double u, double v){
    const auto *px = src.colorData();
    const std::size_t w = src.width(), h = src.height();
    double fu = u * w - 0.5;
    double fv = v * h - 0.5;
    fu = std::clamp(fu, 0.0, static_cast<double>(w - 1));
    fv = std::clamp(fv, 0.0, static_cast<double>(h - 1));
    const std::size_t x0 = static_cast<std::size_t>(fu);
    const std::size_t y0 = static_cast<std::size_t>(fv);
    const std::size_t x1 = std::min(x0 + 1, w - 1);
    const std::size_t y1 = std::min(y0 + 1, h - 1);
    const double tx = fu - x0, ty = fv - y0;
    Color3f out{};
    const uint32_t c00 = px[y0 * w + x0], c10 = px[y0 * w + x1];
    const uint32_t c01 = px[y1 * w + x0], c11 = px[y1 * w + x1];
    for(int ch = 0; ch < 3; ch++){
        const int sh = ch * 8;
        const double a = ((c00 >> sh) & 0xFF) * (1 - tx) + ((c10 >> sh) & 0xFF) * tx;
        const double b = ((c01 >> sh) & 0xFF) * (1 - tx) + ((c11 >> sh) & 0xFF) * tx;
        const double val = a * (1 - ty) + b * ty;
        if(ch == 0){ out.r = val; } else if(ch == 1){ out.g = val; } else { out.b = val; }
    }
    return out;
}

inline void GaussianBlur(FrameBuffer &img, int radius){
    if(radius <= 0){ return; }
    const std::size_t w = img.width(), h = img.height();
    std::vector<uint32_t> tmp(w * h);
    const auto *in = img.colorData();
    // horizontal
    for(std::size_t y = 0; y < h; y++){
        for(std::size_t x = 0; x < w; x++){
            double acc[3]{};
            double wsum = 0;
            for(int k = -radius; k <= radius; k++){
                const std::size_t xx = std::clamp<std::int64_t>(
                    static_cast<std::int64_t>(x) + k, 0,
                    static_cast<std::int64_t>(w - 1));
                const double wt = std::exp(-static_cast<double>(k * k)
                    / (2.0 * radius * radius * 0.25));
                const uint32_t c = in[y * w + xx];
                acc[0] += ((c >> 16) & 0xFF) * wt;
                acc[1] += ((c >> 8) & 0xFF) * wt;
                acc[2] += (c & 0xFF) * wt;
                wsum += wt;
            }
            tmp[y * w + x] = 0xFF000000u
                | (static_cast<uint32_t>(acc[0] / wsum + 0.5) << 16)
                | (static_cast<uint32_t>(acc[1] / wsum + 0.5) << 8)
                | static_cast<uint32_t>(acc[2] / wsum + 0.5);
        }
    }
    // vertical
    for(std::size_t y = 0; y < h; y++){
        for(std::size_t x = 0; x < w; x++){
            double acc[3]{};
            double wsum = 0;
            for(int k = -radius; k <= radius; k++){
                const std::size_t yy = std::clamp<std::int64_t>(
                    static_cast<std::int64_t>(y) + k, 0,
                    static_cast<std::int64_t>(h - 1));
                const double wt = std::exp(-static_cast<double>(k * k)
                    / (2.0 * radius * radius * 0.25));
                const uint32_t c = tmp[yy * w + x];
                acc[0] += ((c >> 16) & 0xFF) * wt;
                acc[1] += ((c >> 8) & 0xFF) * wt;
                acc[2] += (c & 0xFF) * wt;
                wsum += wt;
            }
            img.setPixel(x, y, 0xFF000000u
                | (static_cast<uint32_t>(acc[0] / wsum + 0.5) << 16)
                | (static_cast<uint32_t>(acc[1] / wsum + 0.5) << 8)
                | static_cast<uint32_t>(acc[2] / wsum + 0.5), -2.0f);
        }
    }
}

inline void ExtractBright(const FrameBuffer &src, FrameBuffer &dst, float threshold){
    RunPass(src, dst, [threshold](double u, double v, const FrameBuffer &s){
        const Color3f c = fetchRGB(s, u, v);
        const double lum = 0.2126 * c.r + 0.7152 * c.g + 0.0722 * c.b;
        const double k = lum > threshold ? (lum - threshold) / std::max(1e-3, lum) : 0.0;
        return Color32{static_cast<int32_t>(std::min(255.0, c.r * k)),
                       static_cast<int32_t>(std::min(255.0, c.g * k)),
                       static_cast<int32_t>(std::min(255.0, c.b * k)), 255};
    });
}

enum class ToneMapMode{
    Clamp,
    Reinhard,
    ACES,
};

// sample scene with simulated over-bright factor then tonemap
inline void ToneMappedView(FrameBuffer &dst, const FrameBuffer &src,
                           float exposureBoost, ToneMapMode mode){
    RunPass(src, dst, [exposureBoost, mode](double u, double v, const FrameBuffer &s){
        Color3f c = fetchRGB(s, u, v);
        c.r *= exposureBoost; c.g *= exposureBoost; c.b *= exposureBoost;
        auto to255 = [](double x){
            return static_cast<int32_t>(std::clamp(x, 0.0, 1.0) * 255.0 + 0.5);
        };
        if(mode == ToneMapMode::Clamp){
            return Color32{to255(c.r / 255.0), to255(c.g / 255.0),
                           to255(c.b / 255.0), 255};
        }
        double r, g, bl;
        if(mode == ToneMapMode::Reinhard){
            r = c.r / (c.r + 255.0) ;
            g = c.g / (c.g + 255.0);
            bl = c.b / (c.b + 255.0);
        }else{
            const double a = 2.51, b = 0.03, cc = 2.43, d = 0.59, e = 0.14;
            auto aces = [&](double x){
                const double xv = x / 255.0;
                return std::clamp(xv * (a * xv + b) / (xv * (cc * xv + d) + e), 0.0, 1.0);
            };
            r = aces(c.r); g = aces(c.g); bl = aces(c.b);
        }
        return Color32{to255(r), to255(g), to255(bl), 255};
    });
}

inline void AdditiveBlend(FrameBuffer &base, const FrameBuffer &layer){
    const std::size_t w = base.width(), h = base.height();
    const auto *lp = layer.colorData();
    for(std::size_t y = 0; y < h; y++){
        for(std::size_t x = 0; x < w; x++){
            const uint32_t b = base.colorData()[y * w + x];
            const uint32_t l = lp[y * w + x];
            const uint32_t nr = std::min(255u, ((b >> 16) & 0xFF) + ((l >> 16) & 0xFF));
            const uint32_t ng = std::min(255u, ((b >> 8) & 0xFF) + ((l >> 8) & 0xFF));
            const uint32_t nb = std::min(255u, (b & 0xFF) + (l & 0xFF));
            base.setPixel(x, y, 0xFF000000u | (nr << 16) | (ng << 8) | nb, -2.0f);
        }
    }
}

// Equirectangular HDR skybox drawn behind everything
inline void DrawEquirectSky(FrameBuffer &fb, const Camera &cam,
                            const HDRImage &hdr, float exposure = 1.0f){
    if(!hdr.valid()){ return; }
    const Vector3DBase<double> fwd = cam.forward().normalize();
    const Vector3DBase<double> right = cam.right().normalize();
    const Vector3DBase<double> up = right.mul(fwd).normalize();
    const double aspect = static_cast<double>(fb.width()) / fb.height();
    const double tanH = std::tan(M_PI / 6);
    const double tanV = tanH / aspect;
    const std::size_t w = fb.width(), h = fb.height();
    for(std::size_t y = 0; y < h; y++){
        const double ny = -(2.0 * (static_cast<double>(y) + 0.5) / h - 1.0);
        for(std::size_t x = 0; x < w; x++){
            const double nx = 2.0 * (static_cast<double>(x) + 0.5) / w - 1.0;
            Vector3DBase<double> dir{
                fwd.x + right.x * nx * tanH + up.x * ny * tanV,
                fwd.y + right.y * nx * tanH + up.y * ny * tanV,
                fwd.z + right.z * nx * tanH + up.z * ny * tanV};
            dir = dir.normalize();
            const double theta = std::atan2(dir.z, dir.x);
            const double phi = std::asin(std::clamp(dir.y, -1.0, 1.0));
            double uu = theta / (2.0 * M_PI) + 0.5;
            double vv = 0.5 - phi / M_PI;
            uu = std::clamp(uu, 0.0, 0.999999);
            vv = std::clamp(vv, 0.0, 0.999999);
            const int hx = std::min(static_cast<int>(uu * hdr.width),
                                    hdr.width - 1);
            const int hy = std::min(static_cast<int>(vv * hdr.height),
                                    hdr.height - 1);
            const std::size_t idx = (static_cast<std::size_t>(hy)
                * hdr.width + hx) * 3;
            float r = hdr.rgb[idx] * exposure;
            float g = hdr.rgb[idx + 1] * exposure;
            float b = hdr.rgb[idx + 2] * exposure;
            auto tone = [](float c){
                return static_cast<int32_t>(
                    std::clamp(c / (c + 1.0f), 0.0f, 1.0f) * 255.0f + 0.5f);
            };
            const Color32 sc{tone(r), tone(g), tone(b), 255};
            fb.setPixel(x, y,
                (static_cast<uint32_t>(sc.a) << 24)
                    | (static_cast<uint32_t>(sc.r) << 16)
                    | (static_cast<uint32_t>(sc.g) << 8)
                    | static_cast<uint32_t>(sc.b), -2.0f);
        }
    }
}

}
