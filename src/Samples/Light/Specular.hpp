#pragma once

#include "../SceneUtil.hpp"
#include "LightUtil.hpp"

#include <cmath>

namespace SGE::Samples {

class SpecularLightScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        LightingRig rig{};
        rig.ambient = 0.06f;
        rig.specularStrength = 1.0f;
        rig.shininess = 64.0f;
        DirectionalLight key{};
        key.direction = Vector3DBase<double>{-0.4, 0.7, -1.0};
        key.color = ColorFlt{1, 1, 1};
        rig.directional.push_back(key);
        PointLight p{};
        p.position = Vector3DBase<double>{2.0, 3.5, -2.0};
        p.range = 30.0;
        rig.point.push_back(p);
        Rasterizer rz{fb};
        drawLitCubeWall(app, rig, rz, app.angle());
        drawLightMarker(app, rz, Vector3DBase<double>{2.0, 3.5, -2.0},
            Color32{255, 250, 230, 255});
    }
    const char *name() const override { return "Specular Highlights"; }
    const char *group() const override { return "Light"; }
};

}
