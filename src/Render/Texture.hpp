#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

enum class TextureFilter{
    Nearest,
    Bilinear,
};

enum class TextureWrap{
    Repeat,
    Clamp,
};

class Texture{
public:
    Texture() = default;

    Texture(std::size_t w, std::size_t h, const uint32_t *bgra);

    static Texture loadFromFile(const std::string &path);

    uint32_t sample(double u, double v,
                    TextureFilter filter = TextureFilter::Nearest,
                    TextureWrap wrap = TextureWrap::Repeat) const;

    std::size_t width() const{ return m_w; }
    std::size_t height() const{ return m_h; }

private:
    uint32_t fetchTexel(int tx, int ty, TextureWrap wrap) const;

    std::size_t m_w{};
    std::size_t m_h{};
    std::vector<uint32_t> m_pixels{};
};
