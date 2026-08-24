#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>

namespace SGE::Samples {

class TriangleScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        Rasterizer rz{fb};
        // NDC (-1..1) -> screen
        auto ndc = [](double nx, double ny){
            return std::pair<int,int>{
                static_cast<int>((nx * 0.5 + 0.5) * 800),
                static_cast<int>((-(ny * 0.5) + 0.5) * 600)};
        };
        auto [x0, y0] = ndc(0.0, 0.5);
        auto [x1, y1] = ndc(0.5, -0.5);
        auto [x2, y2] = ndc(-0.5, -0.5);
        ScreenVertex v[3] = {};
        v[0] = {static_cast<double>(x0), static_cast<double>(y0), 0.5f, 1};
        v[1] = {static_cast<double>(x1), static_cast<double>(y1), 0.5f, 1};
        v[2] = {static_cast<double>(x2), static_cast<double>(y2), 0.5f, 1};
        v[0].color = Color32{255, 0, 0, 255};
        v[1].color = Color32{0, 0, 255, 255};
        v[2].color = Color32{0, 255, 0, 255};
        rz.drawTriangleSolid(v[0], v[1], v[2]);
    }
    void drawUi(Application &) override {
        ImGui::Text("red / blue / green gradient triangle");
    }
    const char *name() const override { return "Triangle (vertex color)"; }
    const char *group() const override { return "Base"; }
};

}
