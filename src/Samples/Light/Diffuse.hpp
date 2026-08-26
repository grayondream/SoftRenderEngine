#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>

namespace SGE::Samples {

class DiffuseLightScene final : public IScene {
    float m_lightColor[4]{1.0f, 1.0f, 1.0f, 1.0f};
    void tintRig(LightingRig &rig){
        const ColorFlt t{m_lightColor[0], m_lightColor[1],
                         m_lightColor[2], 1.0f};
        for(auto &dl : rig.directional){ dl.color = dl.color * t; }
        for(auto &pl : rig.point){ pl.color = pl.color * t; }
        for(auto &sl : rig.spot){ sl.color = sl.color * t; }
        rig.ambientColor = ColorFlt{rig.ambientColor.r * t.r,
            rig.ambientColor.g * t.g, rig.ambientColor.b * t.b, 1.0f};
    }
public:
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        Rasterizer rz{fb};
        static Object4D sphere = SGE::Render::MakeSphere(1.0, 36, 18);
        Texture tint(1, 1, std::vector<uint32_t>{0xFFFF7F4F}.data());
        const Vector3DBase<double> lp{1.0, 1.0, 1.5};
        LightingRig rig{};
        rig.ambient = 0.3f;
        rig.specularStrength = 0.0f;
        PointLight p{};
        p.position = lp;
        p.range = 100.0;
        rig.point.push_back(p);
        tintRig(rig);
        ShadingContext ctx{&rig, app.camera().position};
        auto sm = SGE::Math::translation(0.0, 0.0, 0.0);
        auto snrm = SGE::Math::normalMatrix(sm);
        auto st = Pipeline::projectObject(sphere, sm,
            refViewProj(app.camera()), snrm, g_renderW, g_renderH);
        for(auto &t : st){
            rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2], tint, &ctx);
        }
        drawLamp(app, rz, lp);
    }
    void drawUi(Application &) override {
        ImGui::Begin("Settings");
        ImGui::Text("Color Picker with Alpha:");
        ImGui::ColorEdit4("Color with Alpha", m_lightColor);
        ImGui::End();
    }
    const char *name() const override { return "Diffuse Light"; }
    const char *group() const override { return "Light"; }
};

}
