#pragma once

#include "../SceneUtil.hpp"

#include <cstddef>

namespace SGE::Samples {

class ShowcaseRayTraceScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        auto &cam = app.camera();
        auto &rig = app.rig();
        auto &rtBuffer = app.rtBuffer();

        fb.clear(0xFF000000u);
        SGE::Render::RayTraceOptions opt{};
        opt.maxDepth = 3;
        opt.background = Color32{25, 28, 40, 255};
        rtBuffer.clear();
        const std::size_t targetW[3] = {100, 200, 400};
        const std::size_t targetH[3] = {75, 150, 300};
        if(rtBuffer.width() != targetW[app.rtQuality()]){
            rtBuffer = FrameBuffer{targetW[app.rtQuality()], targetH[app.rtQuality()]};
        }
        SGE::Render::RayTracer tracer{rtBuffer};
        tracer.render(app.rtScene(), cam, rig, opt);
        const auto *srcRT = rtBuffer.colorData();
        const std::size_t rw = rtBuffer.width(), rh = rtBuffer.height();
        for(std::size_t yy = 0; yy < 600; yy++){
            const std::size_t sy2 = yy * rh / 600;
            for(std::size_t xx = 0; xx < 800; xx++){
                const std::size_t sx2 = xx * rw / 800;
                fb.setPixel(xx, yy, srcRT[sy2 * rw + sx2], -2.0f);
            }
        }
    }
    void drawUi(Application &app) override {
        const char *qs[] = {"100x75", "200x150", "400x300"};
        ImGui::Combo("RT Quality", &app.rtQuality(), qs, 3);
        ImGui::SliderFloat("Cone Intensity", &app.spotConeIntensity(), 0.0f, 3.0f);
    }
    const char *name() const override { return "Light Cone Ray Tracing"; }
    const char *group() const override { return "Showcase"; }
};

}
