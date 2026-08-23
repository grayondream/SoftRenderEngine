#pragma once

#include "../SceneUtil.hpp"
#include "LightUtil.hpp"

#include <cmath>

namespace SGE::Samples {

class SpotLightScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        auto fog = defaultFog(app);
        fb.clear(0xFF101018u);
        LightingRig rig{};
        rig.ambient = 0.05f;
        rig.specularStrength = 0.5f;
        const double a = app.angle();
        SpotLight spot{};
        spot.position = Vector3DBase<double>{0.0, 4.5, -2.0};
        spot.direction = Vector3DBase<double>{std::sin(a), -1.0, std::cos(a)};
        spot.range = 30.0;
        spot.cutoffCos = 0.90;
        rig.spot.push_back(spot);

        Rasterizer rz{fb};
        ShadingContext ctx{&rig, app.camera().position,
            app.fogEnabled() ? &fog : nullptr};
        Object4D ground = makePlane(9.0, -2.0,
            Vector3DBase<double>{0, 1, 0}, Color32{205, 205, 210, 255}, 18.0);
        auto gm = SGE::Math::translation(0.0, 0.0, 0.0);
        auto gnrm = SGE::Math::normalMatrix(gm);
        SGE::Render::TileRenderer tiled{fb};
        auto gt = Pipeline::projectObject(ground, gm, defaultViewProj(app), gnrm, 800, 600);
        tiled.drawTextured(gt, app.checker(), &ctx);
        drawLitCubeWall(app, rig, rz, app.angle());
        drawLightMarker(app, rz, spot.position, Color32{255, 255, 230, 255});
    }
    const char *name() const override { return "Spot Light (soft cone)"; }
    const char *group() const override { return "Light"; }
};

}
