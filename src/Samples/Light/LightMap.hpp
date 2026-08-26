#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>

namespace SGE::Samples {

class LightMapScene final : public IScene {
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
        static Texture diffuse = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/container2.jpg");
        static Texture specular = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/container2_specular.jpg");
        const double t = app.angle();
        const Vector3DBase<double> lp{5.0 * std::sin(t), 0.0, 5.0 * std::cos(t)};
        LightingRig rig{};
        rig.ambient = 0.12f;
        rig.specularStrength = 0.6f;
        rig.shininess = 1.0f;
        PointLight p{};
        p.position = lp;
        p.color = ColorFlt{0.6f, 0.6f, 0.6f, 1.0f};
        p.range = 100.0;
        rig.point.push_back(p);
        tintRig(rig);
        ShadingContext ctx{&rig, app.camera().position};
        ctx.specTex = &specular;
        Object4D cube = unitCube(app);
        auto cm = SGE::Math::translation(1.0, 0.0, 0.0);
        auto cnrm = SGE::Math::normalMatrix(cm);
        auto ct = Pipeline::projectObject(cube, cm,
            refViewProj(app.camera()), cnrm, g_renderW, g_renderH);
        for(auto &tr : ct){
            rz.drawTriangleTextured(tr.v[0], tr.v[1], tr.v[2],
                                    diffuse, &ctx);
        }
        drawLamp(app, rz, lp);
    }
    void drawUi(Application &) override {
        ImGui::Begin("OpenGL");
        ImGui::Text("Color Picker with Alpha:");
        ImGui::ColorEdit4("Color with Alpha", m_lightColor);
        ImGui::End();
    }
    const char *name() const override { return "Light Map (container2)"; }
    const char *group() const override { return "Light"; }
};

}
