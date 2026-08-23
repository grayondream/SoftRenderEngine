#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

enum class TextureFilter{
    Nearest,
    Bilinear,
    Trilinear,
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
                    TextureFilter filter = TextureFilter::Bilinear,
                    TextureWrap wrap = TextureWrap::Repeat) const;

    // lod is a continuous mipmap level; blends bilinear samples of adjacent levels
    uint32_t sampleTrilinear(double u, double v, float lod, TextureWrap wrap) const;

    void buildMipChain();

    std::size_t width() const{ return m_w; }
    std::size_t height() const{ return m_h; }
    std::size_t mipCount() const{ return m_mips.size(); }

private:
    using Level = std::vector<uint32_t>;

    uint32_t fetchTexelFrom(const Level &lv, std::size_t lw, std::size_t lh,
                            int tx, int ty, TextureWrap wrap) const;
    uint32_t sampleLevel(std::size_t level, double u, double v,
                         TextureFilter filter, TextureWrap wrap) const;

    std::size_t m_w{};
    std::size_t m_h{};
    Level m_pixels{};
    std::vector<Level> m_mips{};   // m_mips[0] mirrors m_pixels
};
