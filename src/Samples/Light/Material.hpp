#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>

namespace SGE::Samples {

class MaterialScene final : public IScene {
public:
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 6.0);
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        Rasterizer rz{fb};
        static Object4D sphere = SGE::Render::MakeSphere(1.0, 36, 18);
        const int count = 5;
        const double t = app.angle();
        const Vector3DBase<double> lp{5.0 * std::sin(t), 0.0, 5.0 * std::cos(t)};
        const auto &cam = app.camera();
        const auto vp = refViewProj(cam);
        Texture tint(1, 1, std::vector<uint32_t>{0xFFFF7F4F}.data());
        for(int i = 0; i < count; i++){
            const double v = static_cast<double>(i + 1) / count;
            LightingRig rig{};
            rig.ambient = static_cast<float>(3.0 * v * v);
            rig.specularStrength = static_cast<float>(3.0 * v);
            rig.shininess = 1.0f;
            PointLight p{};
            p.position = lp;
            p.range = 100.0;
            rig.point.push_back(p);
            ShadingContext ctx{&rig, cam.position};
            const double x = (i - count / 2.0) * 2.5;
            auto sm = SGE::Math::translation(x, 0.0, 0.0);
            auto snrm = SGE::Math::normalMatrix(sm);
            auto st = Pipeline::projectObject(sphere, sm, vp, snrm, g_renderW, g_renderH);
            for(auto &tr : st){
                rz.drawTriangleTextured(tr.v[0], tr.v[1], tr.v[2],
                                        tint, &ctx);
            }
        }
        drawLamp(app, rz, lp);
    }
    void drawUi(Application &) override {
        ImGui::Text("brightness ramp x5 (cam z=6)");
    }
    const char *name() const override { return "Material Strength Ramp"; }
    const char *group() const override { return "Light"; }
};

}
