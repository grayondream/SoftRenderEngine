#pragma once

#include "Texture.hpp"

#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include "../math/Vector/Vector3DBase.hpp"
#include "GeoObject/Color.hpp"

namespace SGE::Render {

struct HDRImage {
    int width{0};
    int height{0};
    std::vector<float> rgb;
    [[nodiscard]] bool valid() const { return width > 0 && height > 0 && !rgb.empty(); }
};

class ImageLoader {
public:
    static Texture loadTexture(const std::string &path, bool srgbAware = true);
    static HDRImage loadHdr(const std::string &path);
};

struct Color3f{
    double r{}, g{}, b{};
};

inline Color3f SampleEquirect(const HDRImage &hdr,
                              const Vector3DBase<double> &dir, float exposure = 1.0f){
    if(!hdr.valid()){ return {}; }
    const double theta = std::atan2(dir.z, dir.x);
    const double phi = std::asin(std::clamp(dir.y, -1.0, 1.0));
    double uu = theta / (2.0 * M_PI) + 0.5;
    double vv = 0.5 - phi / M_PI;
    uu = std::clamp(uu, 0.0, 0.999999);
    vv = std::clamp(vv, 0.0, 0.999999);
    const int hx = std::min(static_cast<int>(uu * hdr.width), hdr.width - 1);
    const int hy = std::min(static_cast<int>(vv * hdr.height), hdr.height - 1);
    const std::size_t idx = (static_cast<std::size_t>(hy) * hdr.width + hx) * 3;
    return {hdr.rgb[idx] * exposure * 255.0,
            hdr.rgb[idx + 1] * exposure * 255.0,
            hdr.rgb[idx + 2] * exposure * 255.0};
}

// Precomputes a low-res cosine-weighted irradiance map (equirect) by
// integrating the hemisphere around each output direction. Mirrors the
// reference irradiance cubemap convolution (sampleDelta-style integration).
inline HDRImage ComputeIrradiance(const HDRImage &env,
                                  int outW = 64, int outH = 32,
                                  int /*samples*/ = 192){
    HDRImage out;
    if(!env.valid()){ return out; }
    out.width = outW;
    out.height = outH;
    out.rgb.assign(static_cast<std::size_t>(outW) * outH * 3, 0.0f);
    const double sampleDelta = 0.025;
    for(int y = 0; y < outH; y++){
        const double phi = M_PI * (y + 0.5) / outH;   // 0..pi
        const double sinTheta = std::sin(phi);
        const double cosTheta = std::cos(phi);
        for(int x = 0; x < outW; x++){
            const double theta = 2.0 * M_PI * (x + 0.5) / outW;
            Vector3DBase<double> N{
                sinTheta * std::cos(theta), cosTheta,
                sinTheta * std::sin(theta)};
            Vector3DBase<double> up{0, 1, 0};
            if(std::abs(cosTheta) > 0.999){
                up = Vector3DBase<double>{1, 0, 0};
            }
            Vector3DBase<double> right{up.y * N.z - up.z * N.y,
                up.z * N.x - up.x * N.z,
                up.x * N.y - up.y * N.x};
            right.normalize();
            Vector3DBase<double> fwd{N.y * right.z - N.z * right.y,
                N.z * right.x - N.x * right.z,
                N.x * right.y - N.y * right.x};
            double r = 0, g = 0, b = 0, wsum = 0;
            for(double ph = 0.0; ph < M_PI / 2; ph += sampleDelta){
                for(double th = 0.0; th < 2.0 * M_PI;
                    th += sampleDelta){
                    const double sp = std::sin(ph);
                    Vector3DBase<double> dir{
                        (sp * std::cos(th)) * right.x
                            + cosTheta * N.x
                            + (sp * std::sin(th)) * fwd.x,
                        (sp * std::cos(th)) * right.y
                            + cosTheta * N.y
                            + (sp * std::sin(th)) * fwd.y,
                        (sp * std::cos(th)) * right.z
                            + cosTheta * N.z
                            + (sp * std::sin(th)) * fwd.z};
                    dir = dir.normalize();
                    // sample env equirect (reuse mapping)
                    const double th2 = std::atan2(dir.z, dir.x);
                    const double ph2 = std::asin(
                        std::clamp(dir.y, -1.0, 1.0));
                    double uu = th2 / (2.0 * M_PI) + 0.5;
                    double vv = 0.5 - ph2 / M_PI;
                    uu = std::clamp(uu, 0.0, 0.999999);
                    vv = std::clamp(vv, 0.0, 0.999999);
                    const int hx = std::min(
                        static_cast<int>(uu * env.width),
                        env.width - 1);
                    const int hy = std::min(
                        static_cast<int>(vv * env.height),
                        env.height - 1);
                    const std::size_t idx =
                        (static_cast<std::size_t>(hy) * env.width
                         + hx) * 3;
                    const double wgt = std::cos(ph) * sp;
                    r += env.rgb[idx] * wgt;
                    g += env.rgb[idx + 1] * wgt;
                    b += env.rgb[idx + 2] * wgt;
                    wsum += wgt;
                }
            }
            const std::size_t oidx =
                (static_cast<std::size_t>(y) * outW + x) * 3;
            const double invPi = 1.0 / M_PI;
            out.rgb[oidx] = static_cast<float>(r / wsum * invPi);
            out.rgb[oidx + 1] =
                static_cast<float>(g / wsum * invPi);
            out.rgb[oidx + 2] =
                static_cast<float>(b / wsum * invPi);
        }
    }
    return out;
}


// Karis split-sum BRDF analytic fit (scale & bias), reference Brdf.fs equiv
inline Vector3DBase<double> EnvBRDFApprox(double NoV, double rough,
                                          double f0){
    const Vector3DBase<double> c0{-1.0, -0.0275, -0.572};
    const Vector3DBase<double> c1{1.0, 0.0325, 1.022};
    const Vector3DBase<double> c2 = c1 + c0;
    (void)f0;
    const double r4 = rough * 4.0;
    const Vector3DBase<double> a{
        std::min(c2.x * r4 + c0.x, -1.0),
        std::min(c2.y * r4 + c0.y, 0.0),
        std::min(c2.z * r4 + c0.z, 1.0)};
    const double t = a.x * a.x * NoV + a.y;
    return Vector3DBase<double>{t * NoV + a.z, 1.0, 0.0};
}

// Builds roughness-blurred equirect mips for specular IBL
inline std::vector<HDRImage> ComputePrefiltered(const HDRImage &env,
                                                int levels = 3){
    std::vector<HDRImage> out;
    if(!env.valid()){ return out; }
    for(int lv = 0; lv < levels; lv++){
        const double rough = 0.15 + 0.35 * lv;
        const int rad = std::max(1,
            static_cast<int>(rough * env.width * 0.05));
        HDRImage img;
        img.width = env.width / ((lv + 1) * 2) + 2;
        img.height = env.height / ((lv + 1) * 2) + 2;
        img.rgb.assign(static_cast<std::size_t>(img.width)
                       * img.height * 3, 0.0f);
        for(int y = 0; y < img.height; y++){
            for(int x = 0; x < img.width; x++){
                double r = 0, g = 0, b = 0, wsum = 0;
                const int bx = x * (env.width / img.width);
                const int by = y * (env.height / img.height);
                for(int oy = -rad; oy <= rad; oy++){
                    for(int ox = -rad; ox <= rad; ox++){
                        const int sx = std::clamp(bx + ox,
                            0, env.width - 1);
                        const int sy = std::clamp(by + oy,
                            0, env.height - 1);
                        const double wgt = std::exp(
                            -(double)(ox * ox + oy * oy)
                            / (2.0 * rad * rad));
                        const std::size_t si =
                            (static_cast<std::size_t>(sy)
                             * env.width + sx) * 3;
                        r += env.rgb[si] * wgt;
                        g += env.rgb[si + 1] * wgt;
                        b += env.rgb[si + 2] * wgt;
                        wsum += wgt;
                    }
                }
                const std::size_t di =
                    (static_cast<std::size_t>(y) * img.width + x) * 3;
                img.rgb[di] = static_cast<float>(r / wsum);
                img.rgb[di + 1] = static_cast<float>(g / wsum);
                img.rgb[di + 2] = static_cast<float>(b / wsum);
            }
        }
        out.push_back(std::move(img));
    }
    return out;
}

inline Color3f SampleEquirectClamped(const HDRImage &hdr,
                                     const Vector3DBase<double> &dir,
                                     float exposure = 1.0f){
    if(!hdr.valid()){ return {}; }
    const double th = std::atan2(dir.z, dir.x);
    const double ph = std::asin(std::clamp(dir.y, -1.0, 1.0));
    const double uu = std::clamp(th / (2.0 * M_PI) + 0.5, 0.0, 0.999999);
    const double vv = std::clamp(0.5 - ph / M_PI, 0.0, 0.999999);
    const int hx = std::min(static_cast<int>(uu * hdr.width),
                            hdr.width - 1);
    const int hy = std::min(static_cast<int>(vv * hdr.height),
                            hdr.height - 1);
    const std::size_t idx =
        (static_cast<std::size_t>(hy) * hdr.width + hx) * 3;
    return {hdr.rgb[idx] * exposure * 255.0,
            hdr.rgb[idx + 1] * exposure * 255.0,
            hdr.rgb[idx + 2] * exposure * 255.0};
}


// ---- disk cache for expensive IBL precomputation ----
inline bool SaveHDRImage(const HDRImage &img, const std::string &path){
    FILE *f = std::fopen(path.c_str(), "wb");
    if(!f){ return false; }
    const uint32_t magic = 0x49424C31;  // "IBL1"
    std::fwrite(&magic, sizeof(magic), 1, f);
    std::fwrite(&img.width, sizeof(int), 1, f);
    std::fwrite(&img.height, sizeof(int), 1, f);
    std::fwrite(img.rgb.data(), sizeof(float), img.rgb.size(), f);
    std::fclose(f);
    return true;
}

inline HDRImage LoadHDRImageCached(const std::string &path){
    HDRImage img;
    FILE *f = std::fopen(path.c_str(), "rb");
    if(!f){ return img; }
    uint32_t magic = 0;
    if(std::fread(&magic, sizeof(magic), 1, f) != 1 || magic != 0x49424C31){
        std::fclose(f);
        return img;
    }
    int w = 0, h = 0;
    if(std::fread(&w, sizeof(int), 1, f) != 1 ||
       std::fread(&h, sizeof(int), 1, f) != 1 || w <= 0 || h <= 0){
        std::fclose(f);
        return img;
    }
    img.width = w;
    img.height = h;
    img.rgb.resize(static_cast<std::size_t>(w) * h * 3);
    if(std::fread(img.rgb.data(), sizeof(float), img.rgb.size(), f)
       != img.rgb.size()){
        img = HDRImage{};
    }
    std::fclose(f);
    return img;
}

// Convenience: compute-or-load cached irradiance
inline HDRImage GetIrradianceCached(const HDRImage &env,
                                    const std::string &cachePath){
    auto cached = LoadHDRImageCached(cachePath);
    if(cached.valid()){ return cached; }
    auto computed = ComputeIrradiance(env);
    SaveHDRImage(computed, cachePath);
    return computed;
}


// Convenience: compute-or-load cached prefiltered mip chain
inline std::vector<HDRImage> GetPrefilteredCached(
    const HDRImage &env, const std::string &cacheBase){
    std::vector<HDRImage> out;
    bool all = true;
    for(int i = 0; i < 3; i++){
        auto c = LoadHDRImageCached(
            cacheBase + "_" + std::to_string(i) + ".bin");
        if(!c.valid()){ all = false; break; }
        out.push_back(std::move(c));
    }
    if(all){ return out; }
    out = ComputePrefiltered(env);
    for(int i = 0; i < static_cast<int>(out.size()); i++){
        SaveHDRImage(out[static_cast<std::size_t>(i)],
            cacheBase + "_" + std::to_string(i) + ".bin");
    }
    return out;
}

}

