#pragma once

#include "../SceneUtil.hpp"

#include <cmath>

namespace SGE::Samples {

// GL AppType: SimpleGeometry — reference draws 4 colored "house" points
// via geometry shader (square body + white roof) at screen corners.
class SimpleGeometryScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        Rasterizer rz{fb};
        auto ndc = [](double nx, double ny){
            return std::pair<double, double>{
                (nx * 0.5 + 0.5) * 800.0,
                (-(ny * 0.5) + 0.5) * 600.0};
        };
        // reference point positions & colors
        const double pts[4][2] = {{-0.5, 0.5}, {0.5, 0.5},
                                  {0.5, -0.5}, {-0.5, -0.5}};
        const Color32 cols[4] = {
            Color32{255, 0, 0, 255}, Color32{0, 255, 0, 255},
            Color32{0, 0, 255, 255}, Color32{255, 255, 0, 255}};
        const double off = 0.2;   // house half-size in NDC
        for(int i = 0; i < 4; i++){
            const auto [cxn, cyn] = ndc(pts[i][0], pts[i][1]);
            const double ox = off * 400.0;
            const double oy = off * 300.0;
            const double roof = off * 300.0;
            ScreenVertex bl{cxn - ox, cyn + oy, 0.5f, 1};
            ScreenVertex br{cxn + ox, cyn + oy, 0.5f, 1};
            ScreenVertex tl{cxn - ox, cyn - oy, 0.5f, 1};
            ScreenVertex tr{cxn + ox, cyn - oy, 0.5f, 1};
            ScreenVertex apex{static_cast<double>(cxn),
                cyn - oy - roof, 0.5f, 1};
            bl.color = cols[i]; br.color = cols[i];
            tl.color = cols[i]; tr.color = cols[i];
            apex.color = Color32{255, 255, 255, 255};  // white roof tip
            rz.drawTriangleSolid(bl, tl, tr);
            rz.drawTriangleSolid(bl, tr, br);
            rz.drawTriangleSolid(tl, apex, tr);
        }
    }
    void drawUi(Application &) override {
        ImGui::Begin("OpenGL");
        ImGui::End();
    }
    const char *name() const override { return "Simple Geometry Houses"; }
    const char *group() const override { return "Advanced"; }
};

}
