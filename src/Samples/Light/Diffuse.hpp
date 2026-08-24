#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>

namespace SGE::Samples {

class DiffuseLightScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        Rasterizer rz{fb};
        static Object4D sphere = SGE::Render::MakeSphere(1.0, 36, 18);
        Texture tint(1, 1, std::vector<uint32_t>{0xFF314D80}.data());
        const Vector3DBase<double> lp{1.0, 1.0, 1.5};
        LightingRig rig{};
        rig.ambient = 0.3f;
        rig.specularStrength = 0.0f;
        PointLight p{};
        p.position = lp;
        p.range = 100.0;
        rig.point.push_back(p);
        ShadingContext ctx{&rig, refCamera().position};
        auto sm = SGE::Math::translation(0.0, 0.0, 0.0);
        auto snrm = SGE::Math::normalMatrix(sm);
        auto st = Pipeline::projectObject(sphere, sm,
            refViewProj(refCamera()), snrm, 800, 600);
        for(auto &t : st){
            rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2], tint, &ctx);
        }
        drawLamp(app, rz, lp);
    }
    void drawUi(Application &) override {
        ImGui::Text("light at (1,1,1.5), ambient 0.3");
    }
    const char *name() const override { return "Diffuse Light"; }
    const char *group() const override { return "Light"; }
};

}
