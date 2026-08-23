#pragma once

#include "../SceneUtil.hpp"
#include "LightUtil.hpp"

#include <cmath>

namespace SGE::Samples {

class PointLightScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        LightingRig rig{};
        rig.ambient = 0.04f;
        rig.specularStrength = 0.6f;
        const double a = app.angle();
        PointLight p{};
        p.position = Vector3DBase<double>{4.0 * std::sin(a), 2.5, -1.0 + 4.0 * std::cos(a)};
        p.range = 14.0;
        rig.point.push_back(p);
        Rasterizer rz{fb};
        drawLitCubeWall(app, rig, rz, app.angle());
        drawLightMarker(app, rz, p.position, Color32{255, 240, 200, 255});
    }
    const char *name() const override { return "Point Light (attenuation)"; }
    const char *group() const override { return "Light"; }
};

}
