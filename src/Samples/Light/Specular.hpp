#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>

namespace SGE::Samples {

class SpecularLightScene final : public IScene {
public:
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        Rasterizer rz{fb};
        static Object4D sphere = SGE::Render::MakeSphere(1.0, 36, 18);
        Texture tint(1, 1, std::vector<uint32_t>{0xFF560ED5}.data());
        const double t = app.angle();
        const Vector3DBase<double> lp{5.0 * std::sin(t), 0.0, 5.0 * std::cos(t)};
        LightingRig rig{};
        rig.ambient = 0.1f;
        rig.specularStrength = 0.5f;
        rig.shininess = 32.0f;
        PointLight p{};
        p.position = lp;
        p.range = 100.0;
        rig.point.push_back(p);
        ShadingContext ctx{&rig, refCamera().position};
        auto sm = SGE::Math::translation(0.0, 0.0, 0.0);
        auto snrm = SGE::Math::normalMatrix(sm);
        auto st = Pipeline::projectObject(sphere, sm,
            refViewProj(refCamera()), snrm, 800, 600);
        for(auto &tr : st){
            rz.drawTriangleTextured(tr.v[0], tr.v[1], tr.v[2], tint, &ctx);
        }
        drawLamp(app, rz, lp);
    }
    void drawUi(Application &) override {
        ImGui::Text("orbiting lamp, Phong pow32");
    }
    const char *name() const override { return "Specular Light"; }
    const char *group() const override { return "Light"; }
};

}
