#pragma once

#include "../SceneUtil.hpp"

namespace SGE::Samples {

class TriangleScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        Rasterizer rz{fb};
        ScreenVertex v[3] = {};
        v[0] = {400, 80,  0.5f, 1};  v[0].color = Color32{255, 60, 60, 255};
        v[1] = {120, 520, 0.5f, 1};  v[1].color = Color32{60, 255, 90, 255};
        v[2] = {680, 520, 0.5f, 1};  v[2].color = Color32{70, 90, 255, 255};
        rz.drawTriangleSolid(v[0], v[1], v[2]);
    }
    const char *name() const override { return "Triangle (vertex color)"; }
    const char *group() const override { return "Base"; }
};

}
