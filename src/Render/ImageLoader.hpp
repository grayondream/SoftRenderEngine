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

}
