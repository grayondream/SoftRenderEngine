#pragma once

#include "../SceneUtil.hpp"
#include "LightUtil.hpp"

#include <cmath>

namespace SGE::Samples {

class AmbientLightScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        LightingRig rig{};
        rig.ambient = 0.45f;
        rig.directional.clear();
        Rasterizer rz{fb};
        drawLitCubeWall(app, rig, rz, app.angle());
    }
    const char *name() const override { return "Ambient Light Only"; }
    const char *group() const override { return "Light"; }
};

}
