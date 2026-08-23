#pragma once

#include "../SceneUtil.hpp"
#include "LightUtil.hpp"

#include <cmath>

namespace SGE::Samples {

class DirLightWallScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        LightingRig rig{};
        rig.ambient = 0.08f;
        rig.specularStrength = 0.4f;
        const double a = app.angle() * 0.7;
        DirectionalLight key{};
        key.direction = Vector3DBase<double>{std::sin(a), -0.5, std::cos(a)};
        rig.directional.push_back(key);
        Rasterizer rz{fb};
        drawLitCubeWall(app, rig, rz, app.angle());
    }
    const char *name() const override { return "Directional Light"; }
    const char *group() const override { return "Light"; }
};

}
