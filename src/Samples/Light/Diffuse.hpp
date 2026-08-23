#pragma once

#include "../SceneUtil.hpp"
#include "LightUtil.hpp"

#include <cmath>

namespace SGE::Samples {

class DiffuseLightScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        LightingRig rig{};
        rig.ambient = 0.02f;
        rig.specularStrength = 0.0f;
        DirectionalLight key{};
        key.direction = Vector3DBase<double>{-0.4, 0.7, -1.0};
        key.color = ColorFlt{1, 1, 1};
        rig.directional.push_back(key);
        Rasterizer rz{fb};
        drawLitCubeWall(app, rig, rz, app.angle());
    }
    const char *name() const override { return "Diffuse (Lambert)"; }
    const char *group() const override { return "Light"; }
};

}
