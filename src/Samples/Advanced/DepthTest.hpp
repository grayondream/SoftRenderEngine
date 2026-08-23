#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"
#include "Light/LightUtil.hpp"

#include <cmath>

namespace SGE::Samples {

class DepthTestScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        Rasterizer rz{fb};
        ScreenVertex A[3] = {{200, 480, 0.55f, 1}, {600, 480, 0.55f, 1}, {400, 120, 0.55f, 1}};
        ScreenVertex B[3] = {{260, 500, 0.45f, 1}, {540, 140, 0.30f, 1}, {520, 500, 0.45f, 1}};
        for(auto &v : A){ v.color = Color32{230, 120, 60, 255}; }
        for(auto &v : B){ v.color = Color32{70, 130, 240, 255}; }
        rz.drawTriangleSolid(A[0], A[1], A[2]);
        rz.drawTriangleSolid(B[0], B[1], B[2]);
    }
    void drawUi(Application &) override {
        ImGui::Text("Blue triangle is closer in the overlap");
    }
    const char *name() const override { return "Depth Test Overlap"; }
    const char *group() const override { return "Advanced"; }
};

}
