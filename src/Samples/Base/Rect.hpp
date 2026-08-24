#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>

namespace SGE::Samples {

class RectScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        Rasterizer rz{fb};
        auto ndc = [](double nx, double ny){
            return std::pair<int,int>{
                static_cast<int>((nx * 0.5 + 0.5) * g_renderW),
                static_cast<int>((-(ny * 0.5) + 0.5) * 600)};
        };
        auto [tx, ty] = ndc(0.5, 0.5);      // TR red
        auto [bx, by] = ndc(0.5, -0.5);     // BR blue
        auto [lx, ly] = ndc(-0.5, -0.5);    // BL green
        auto [lx2, ty2] = ndc(-0.5, 0.5);   // TL white
        ScreenVertex TR{static_cast<double>(tx), static_cast<double>(ty), 0.5f, 1},
                       BR{static_cast<double>(bx), static_cast<double>(by), 0.5f, 1},
                       BL{static_cast<double>(lx), static_cast<double>(ly), 0.5f, 1},
                       TL{static_cast<double>(lx2), static_cast<double>(ty2), 0.5f, 1};
        TR.color = Color32{255, 0, 0, 255};
        BR.color = Color32{0, 0, 255, 255};
        BL.color = Color32{0, 255, 0, 255};
        TL.color = Color32{255, 255, 255, 255};
        rz.drawTriangleSolid(TR, BR, BL);
        rz.drawTriangleSolid(BR, TL, BL);
    }
    void drawUi(Application &) override {
        ImGui::Text("four-corner gradient quad");
    }
    const char *name() const override { return "Rect (vertex color)"; }
    const char *group() const override { return "Base"; }
};

}
