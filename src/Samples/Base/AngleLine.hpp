#pragma once

#include "../SceneUtil.hpp"

#include <cmath>

namespace SGE::Samples {

// 30-degree straight line drawn over a reference grid: from the origin,
// a segment along direction (cos30, sin30) plus the x-axis and 60-degree
// companions so the 30-degree slope is immediately readable.
class AngleLineScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        Rasterizer rz{fb};
        const double c = g_renderW * 0.5, r0 = g_renderH * 0.62;
        const double scale = 0.45 * g_renderH;
        const double ca = std::cos(30.0 * M_PI / 180.0);
        const double sa = std::sin(30.0 * M_PI / 180.0);
        // 30-degree hypotenuse (yellow), full length
        ScreenVertex a{}, b{};
        a.x = c; a.y = r0;
        b.x = c + ca * scale;
        b.y = r0 - sa * scale;
        auto yellow = Color32{255, 255, 0, 255};
        a.color = yellow; b.color = yellow;
        rz.drawLine(a, b);
        // horizontal leg (gray)
        ScreenVertex bx{};
        bx.x = c + ca * scale; bx.y = r0;
        auto gray = Color32{120, 120, 120, 255};
        a.color = gray; bx.color = gray;
        rz.drawLine(a, bx);
        // vertical leg (gray) closing the 30-60-90 triangle
        ScreenVertex by{};
        by.x = c + ca * scale; by.y = r0 - sa * scale;
        a.color = gray; bx.x = by.x; bx.y = r0;
        rz.drawLine(bx, by);
        // vertex marker
        ScreenVertex m{};
        m.x = c; m.y = r0;
        auto white = Color32{255, 255, 255, 255};
        for(int dy = -3; dy <= 3; dy++){
            for(int dx = -3; dx <= 3; dx++){
                m.x = c + dx; m.y = r0 + dy;
                m.color = white;
                rz.drawLine(m, m);
            }
        }
    }
    void drawUi(Application &) override {
        ImGui::Begin("Settings");
        ImGui::Text("30-degree line");
        ImGui::End();
    }
    const char *name() const override { return "30-degree Line"; }
    const char *group() const override { return "Base"; }
};

}
