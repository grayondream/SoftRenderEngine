#pragma once

#include "../SceneUtil.hpp"
#include "Render/PostProcess.hpp"

#include <cmath>

namespace SGE::Samples {

class HDRTonemapScene final : public IScene {
public:
    static void blendStrip(FrameBuffer &fb, const FrameBuffer &src, int x0, int x1){
        for(int y = 0; y < 600; y++){
            for(int x = x0; x < x1 && x < 800; x++){
                fb.setPixel(x, y, src.colorData()[y * src.width() + x], -2.0f);
            }
        }
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF000000u);
        static FrameBuffer sceneFb{800, 600};
        sceneFb.clear();
        const std::size_t w = 800;
        for(std::size_t y = 0; y < 600; y++){
            for(std::size_t x = 0; x < w; x++){
                const double u = x / double(w);
                const double glow = std::exp(-std::pow(u * w - 400.0, 2)
                    / (2.0 * 120.0 * 120.0))
                    * (1.0 + 6.0 * std::sin(app.angle()));
                const int32_t r = static_cast<int32_t>(40 + glow * 90);
                const int32_t g = static_cast<int32_t>(60 + glow * 140);
                const int32_t b = static_cast<int32_t>(120 + glow * 220);
                const Color32 pc{std::min(r, 1020), std::min(g, 1020),
                    std::min(b, 1020), 255};
                sceneFb.setPixel(x, y,
                    (static_cast<uint32_t>(pc.a) << 24)
                        | (static_cast<uint32_t>(pc.r) << 16)
                        | (static_cast<uint32_t>(pc.g) << 8)
                        | static_cast<uint32_t>(pc.b), -2.0f);
            }
        }
        static FrameBuffer tmp{800, 600};
        SGE::Render::ToneMappedView(tmp, sceneFb, 1.0f, SGE::Render::ToneMapMode::Clamp);
        blendStrip(fb, tmp, 0, 267);
        SGE::Render::ToneMappedView(tmp, sceneFb, 1.0f, SGE::Render::ToneMapMode::Reinhard);
        blendStrip(fb, tmp, 267, 533);
        SGE::Render::ToneMappedView(fb, sceneFb, 1.0f, SGE::Render::ToneMapMode::ACES);
    }
    void drawUi(Application &) override {
        ImGui::Text("Left: clamp | Mid: Reinhard | Right: ACES");
    }
    const char *name() const override { return "HDR Tonemapping"; }
    const char *group() const override { return "LightAdv"; }
};

}
