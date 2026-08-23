#pragma once
#include "Rasterizer.hpp"
#include "Pipeline.hpp"
#include <thread>
#include <vector>

namespace SGE::Render{

class TileRenderer{
public:
    explicit TileRenderer(FrameBuffer &fb) : m_fb(fb){ }

    void drawTextured(const std::vector<Pipeline::ScreenTriangle> &tris,
                      const Texture &tex, const ShadingContext *ctx,
                      TextureFilter filter = TextureFilter::Bilinear,
                      TextureWrap wrap = TextureWrap::Repeat,
                      unsigned threads = std::thread::hardware_concurrency()){
        const std::size_t W = m_fb.width();
        const std::size_t H = m_fb.height();
        constexpr std::size_t kTile = 64;
        const std::size_t cols = (W + kTile - 1) / kTile;
        const std::size_t rows = (H + kTile - 1) / kTile;
        const std::size_t tileCount = cols * rows;

        std::vector<std::vector<std::uint32_t>> buckets(tileCount);
        for(std::uint32_t i = 0; i < tris.size(); i++){
            const auto &t = tris[i];
            const double minX = std::min({t.v[0].x, t.v[1].x, t.v[2].x});
            const double maxX = std::max({t.v[0].x, t.v[1].x, t.v[2].x});
            const double minY = std::min({t.v[0].y, t.v[1].y, t.v[2].y});
            const double maxY = std::max({t.v[0].y, t.v[1].y, t.v[2].y});
            if(maxX < 0 || maxY < 0) continue;
            if(minX >= static_cast<double>(W) || minY >= static_cast<double>(H)) continue;
            const std::size_t c0 = static_cast<std::size_t>(std::max(0.0, std::floor(minX))) / kTile;
            const std::size_t c1 = std::min(cols - 1, static_cast<std::size_t>(std::max(0.0, std::floor(maxX))) / kTile);
            const std::size_t r0 = static_cast<std::size_t>(std::max(0.0, std::floor(minY))) / kTile;
            const std::size_t r1 = std::min(rows - 1, static_cast<std::size_t>(std::max(0.0, std::floor(maxY))) / kTile);
            for(std::size_t r = r0; r <= r1; r++){
                for(std::size_t c = c0; c <= c1; c++){
                    buckets[r * cols + c].push_back(i);
                }
            }
        }

        if(threads == 0) threads = 1;
        if(threads > tileCount) threads = static_cast<unsigned>(tileCount);

        auto worker = [&](unsigned id){
            Rasterizer rz{m_fb};
            for(std::size_t t = id; t < tileCount; t += threads){
                for(auto idx : buckets[t]){
                    const auto &tri = tris[idx];
                    rz.drawTriangleTextured(tri.v[0], tri.v[1], tri.v[2],
                                            tex, ctx, filter, wrap);
                }
            }
        };

        std::vector<std::thread> pool;
        pool.reserve(threads - 1);
        for(unsigned i = 1; i < threads; i++){
            pool.emplace_back(worker, i);
        }
        worker(0);
        for(auto &th : pool){
            th.join();
        }
    }

private:
    FrameBuffer &m_fb;
};

}
