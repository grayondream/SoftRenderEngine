#pragma once

#include "Texture.hpp"

#include <string>
#include <vector>

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

}
