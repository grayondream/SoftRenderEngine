#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD
#include "ImageLoader.hpp"

#include "stb_image.h"

#include <cmath>

namespace SGE::Render {

Texture ImageLoader::loadTexture(const std::string &path, bool srgbAware) {
    int w = 0, h = 0, comp = 0;
    stbi_uc *data = stbi_load(path.c_str(), &w, &h, &comp, 4);
    if(!data){
        return Texture(1, 1, std::vector<uint32_t>{0xFFFF00FFu}.data());
    }
    const std::size_t n = static_cast<std::size_t>(w) * static_cast<std::size_t>(h);
    std::vector<uint32_t> pixels(n);
    for(std::size_t i = 0; i < n; i++){
        const uint32_t r = data[i * 4 + 0];
        const uint32_t g = data[i * 4 + 1];
        const uint32_t b = data[i * 4 + 2];
        const uint32_t a = data[i * 4 + 3];
        pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }
    stbi_image_free(data);
    (void)srgbAware;
    return Texture(w, h, pixels.data());
}

HDRImage ImageLoader::loadHdr(const std::string &path) {
    HDRImage img;
    int w = 0, h = 0, comp = 0;
    float *data = stbi_loadf(path.c_str(), &w, &h, &comp, 3);
    if(!data){
        return img;
    }
    img.width = w;
    img.height = h;
    img.rgb.resize(static_cast<std::size_t>(w) * static_cast<std::size_t>(h) * 3);
    std::copy(data, data + img.rgb.size(), img.rgb.begin());
    stbi_image_free(data);
    return img;
}

}
