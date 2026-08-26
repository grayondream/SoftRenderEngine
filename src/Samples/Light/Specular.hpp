#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>

namespace SGE::Samples {

class SpecularLightScene final : public IScene {
public:
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
    float m_ambientStrength{0.1f};
    float m_specularStrength{0.5f};
    float m_diffuseStrength{1.0f};
    int m_powTimes{32};
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        Rasterizer rz{fb};
        static Object4D sphere = SGE::Render::MakeSphere(1.0, 36, 18);
        Texture tint(1, 1, std::vector<uint32_t>{0xFFD50E56}.data());
        // fixed key light in front-right-top of the sphere so the lit side
        // and specular highlight always face the viewer (orbiting lamps kept
        // crossing behind the camera and looked like inverted lighting)
        const Vector3DBase<double> lp{1.2, 1.0, 2.0};
        LightingRig rig{};
        rig.ambient = m_ambientStrength;
        rig.specularStrength = m_specularStrength;
        rig.shininess = static_cast<float>(m_powTimes);
        PointLight p{};
        p.position = lp;
        p.color = ColorFlt{m_diffuseStrength, m_diffuseStrength,
            m_diffuseStrength, 1.0f};
        p.range = 100.0;
        rig.point.push_back(p);
        tintRig(rig);
        ShadingContext ctx{&rig, app.camera().position};
        auto sm = SGE::Math::translation(0.0, 0.0, 0.0);
        auto snrm = SGE::Math::normalMatrix(sm);
        auto st = Pipeline::projectObject(sphere, sm,
            refViewProj(app.camera()), snrm, g_renderW, g_renderH);
        for(auto &tr : st){
            rz.drawTriangleTextured(tr.v[0], tr.v[1], tr.v[2], tint, &ctx);
        }
        drawLamp(app, rz, lp);
    }
    void drawUi(Application &) override {
        ImGui::Begin("OpenGL");
        ImGui::Text("Color Picker with Alpha:");
        ImGui::ColorEdit4("Color with Alpha", m_lightColor);
        ImGui::SliderFloat("Ambient Slider", &m_ambientStrength, 0.0f, 1.0f);
        ImGui::SliderFloat("Specular Slider", &m_specularStrength, 0.0f, 1.0f);
        ImGui::SliderFloat("diffuseStrength Slider", &m_diffuseStrength, 0.0f, 1.0f);
        ImGui::SliderInt("Times Slider", &m_powTimes, 0.0f, 512);
        ImGui::End();
    }
    const char *name() const override { return "Specular Light"; }
    const char *group() const override { return "Light"; }
};

}
