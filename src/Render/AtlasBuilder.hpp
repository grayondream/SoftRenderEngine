#pragma once

#include "ImageLoader.hpp"
#include "ObjLoader.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace SGE::Render {

struct AtlasTile {
    double u0{}, v0{}, u1{}, v1{};  // normalized region
};

struct DiffuseAtlas {
    Texture texture{};
    std::vector<AtlasTile> tiles;   // per material index
    bool valid{false};
};

// Builds a single texture atlas from the diffuse maps listed in an MTL.
// Each source map is nearest-resampled into a fixed grid cell.
inline DiffuseAtlas BuildDiffuseAtlas(const ObjMaterialInfo &materials){
    DiffuseAtlas out;
    const int count = static_cast<int>(materials.names.size());
    if(count == 0){ return out; }
    const int cols = (count + 1) / 2;
    const int rows = 2;
    constexpr int kCell = 256;
    const std::size_t aw = static_cast<std::size_t>(cols) * kCell;
    const std::size_t ah = static_cast<std::size_t>(rows) * kCell;
    std::vector<uint32_t> pixels(aw * ah, 0xFF9A9A9Au);

    for(int m = 0; m < count; m++){
        const int cx = m % cols;
        const int cy = m / cols;
        Texture tex = materials.diffuseMap[m].empty()
            ? Texture(1, 1, std::vector<uint32_t>{0xFFB0B0B0u}.data())
            : ImageLoader::loadTexture(
                materials.mtlDir + "/" + materials.diffuseMap[m]);
        // resample into cell (nearest)
        for(int py = 0; py < kCell; py++){
            const int sy = std::min(tex.height() - 1,
                py * tex.height() / kCell);
            for(int px = 0; px < kCell; px++){
                const int sx = std::min(tex.width() - 1,
                    px * tex.width() / kCell);
                pixels[(static_cast<std::size_t>(cy * kCell + py)) * aw
                       + static_cast<std::size_t>(cx * kCell + px)] =
                    tex.sample(
                        (sx + 0.5) / tex.width(),
                        (sy + 0.5) / tex.height(),
                        TextureFilter::Nearest, TextureWrap::Clamp);
            }
        }
        AtlasTile tile{};
        tile.u0 = static_cast<double>(cx) * kCell / aw;
        tile.v0 = static_cast<double>(cy) * kCell / ah;
        tile.u1 = static_cast<double>(cx + 1) * kCell / aw;
        tile.v1 = static_cast<double>(cy + 1) * kCell / ah;
        out.tiles.push_back(tile);
    }
    out.texture = Texture(static_cast<int>(aw), static_cast<int>(ah),
                          pixels.data());
    out.valid = true;
    return out;
}

}
