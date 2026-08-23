#pragma once

#include "../SceneUtil.hpp"
#include "Render/PostProcess.hpp"

#include <cmath>

namespace SGE::Samples {

class GammaScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        // grayscale steps
        static FrameBuffer steps{800, 600};
        steps.clear();
        const int n = 12;
        for(int i = 0; i < n; i++){
            const int v = static_cast<int>(255.0 * i / (n - 1));
            const int x0 = i * 800 / n;
            const int x1 = (i + 1) * 800 / n;
            for(int y = 100; y < 500; y++){
                for(int x = x0; x < x1; x++){
                    const Color32 cc{v, v, v, 255};
                    steps.setPixel(x, y,
                        (static_cast<uint32_t>(cc.a) << 24)
                            | (static_cast<uint32_t>(cc.r) << 16)
                            | (static_cast<uint32_t>(cc.g) << 8)
                            | static_cast<uint32_t>(cc.b), -2.0f);
                }
            }
        }
        if(m_gammaOn){
            SGE::Render::RunPass(steps, fb, [](double u, double v, const FrameBuffer &s){
                const auto c = SGE::Render::fetchRGB(s, u, v);
                auto gam = [](double ch){
                    return static_cast<int32_t>(
                        std::pow(ch / 255.0, 1.0 / 2.2) * 255.0 + 0.5);
                };
                return Color32{gam(c.r), gam(c.g), gam(c.b), 255};
            });
        }else{
            SGE::Render::RunPass(steps, fb, [](double u, double v, const FrameBuffer &s){
                const auto c = SGE::Render::fetchRGB(s, u, v);
                return Color32{static_cast<int32_t>(c.r),
                               static_cast<int32_t>(c.g),
                               static_cast<int32_t>(c.b), 255};
            });
        }
    }
    void drawUi(Application &) override {
        ImGui::Checkbox("Gamma Correct (1/2.2)", &m_gammaOn);
    }
    bool m_gammaOn{true};
    const char *name() const override { return "Gamma Correction"; }
    const char *group() const override { return "LightAdv"; }
};

}
