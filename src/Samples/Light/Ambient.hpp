#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>

namespace SGE::Samples {

class AmbientLightScene final : public IScene {
public:
    float m_lightColor[3]{1.0f, 1.0f, 1.0f};
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        Rasterizer rz{fb};
        // reference: FragColor = 0.2 * lightColor * objectColor (copper)
        static Object4D sphere = SGE::Render::MakeSphere(1.0, 36, 18);
        Texture tint(1, 1, std::vector<uint32_t>{0xFFFF7F4F}.data());
        LightingRig rig{};
        rig.ambient = 0.2f;
        rig.ambientColor = ColorFlt{m_lightColor[0], m_lightColor[1],
                                    m_lightColor[2], 1.0f};
        rig.directional.clear();
        rig.point.clear();
        ShadingContext ctx{&rig, refCamera().position};
        auto sm = SGE::Math::translation(0.0, 0.0, 0.0);
        auto snrm = SGE::Math::normalMatrix(sm);
        auto st = Pipeline::projectObject(sphere, sm,
            refViewProj(refCamera()), snrm, g_renderW, g_renderH);
        for(auto &t : st){
            rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2], tint, &ctx);
        }
        drawLamp(app, rz, Vector3DBase<double>{1, 1, 1});
    }
    void drawUi(Application &) override {
        ImGui::ColorEdit3("Light Color", m_lightColor);
        ImGui::Text("0.2 * lightColor * copper sphere");
    }
    const char *name() const override { return "Ambient Light"; }
    const char *group() const override { return "Light"; }
};

}
