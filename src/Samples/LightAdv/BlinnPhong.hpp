#pragma once

#include "../SceneUtil.hpp"
#include "Render/Shadow.hpp"

#include <cmath>

namespace SGE::Samples {

class BlinnPhongScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        LightingRig base{};
        base.ambient = 0.10f;
        PointLight p{};
        p.position = Vector3DBase<double>{
            2.2 * std::sin(app.angle()), 2.4, -1.0 + 2.2 * std::cos(app.angle())};
        p.range = 25.0;
        base.point.push_back(p);
        Rasterizer rz{fb};
        drawLightMarker(app, rz, p.position, Color32{255, 245, 210, 255});
        // left: classic Phong (shininess 8), right: Blinn-Phong (shininess 48)
        struct Half{ float shininess; float spec; };
        const Half halves[2] = {{8.0f, 1.0f}, {48.0f, 1.0f}};
        for(int side = 0; side < 2; side++){
            LightingRig rig = base;
            rig.shininess = halves[side].shininess;
            rig.specularStrength = halves[side].spec;
            Object4D sphere = SGE::Render::MakeSphere(1.05, 26, 18);
            auto sm = SGE::Math::translation(-1.6 + side * 3.2, 1.3, 3.0);
            auto snrm = SGE::Math::normalMatrix(sm);
            ShadingContext ctx{&rig, app.camera().position};
            SGE::Render::TileRenderer tiled{fb};
            auto st = Pipeline::projectObject(sphere, sm,
                defaultViewProj(app), snrm, 800, 600);
            tiled.drawTextured(st, app.checker(), &ctx);
        }
    }
    void drawUi(Application &) override {
        ImGui::Text("Left: Phong (shininess 8) | Right: Blinn-Phong (48)");
    }
    const char *name() const override { return "Phong vs Blinn-Phong highlight"; }
    const char *group() const override { return "LightAdv"; }
};

}
