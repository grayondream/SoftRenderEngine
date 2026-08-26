#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>
#include <vector>

namespace SGE::Samples {

class SpotLightScene final : public IScene {
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
        Object4D cubeProto = unitCube(app);
        const double t = app.angle();
        const Vector3DBase<double> lp{5.0 * std::sin(t), 0.0, 5.0 * std::cos(t)};
        LightingRig rig{};
        // spotlight following -Z: inner cos12.5 outer cos17.5
        rig.ambient = 0.2f;
        rig.specularStrength = 1.0f;
        rig.shininess = 1.0f;
        SpotLight spot{};
        spot.position = lp;
        spot.direction = Vector3DBase<double>{0.0, 0.0, -1.0};
        spot.cutoffCos = std::cos(12.5 * M_PI / 180.0);
        spot.outerCutoffCos = std::cos(17.5 * M_PI / 180.0);
        spot.color = ColorFlt{0.5f, 0.5f, 0.5f, 1.0f};
        spot.range = 60.0;
        rig.spot.push_back(spot);
        tintRig(rig);
        ShadingContext ctx{&rig, app.camera().position};
        ctx.specTex = &specular;
        SGE::Render::TileRenderer tiled{fb};
        const auto vp = refViewProj(app.camera());
        const double rotA = t;  // radians(20 * time)
        int drawn = 0;
        for(int gi = 0; gi < 5 && drawn < m_count; gi++){
            for(int gj = 0; gj < 5 && drawn < m_count; gj++){
                for(int gk = 0; gk < 5 && drawn < m_count; gk++){
                    const double px = (gi - 2) * 2.0;
                    const double py = (gj - 2) * 2.0;
                    const double pz = (gk - 2) * 2.0 - 10.0;
                    auto cm = SGE::Math::translation(px, py, pz)
                        .mul(SGE::Math::rotationY(rotA))
                        .mul(SGE::Math::rotationX(rotA * 0.3))
                        .mul(SGE::Math::rotationZ(rotA * 0.5));
                    auto cnrm = SGE::Math::normalMatrix(cm);
                    auto ct = Pipeline::projectObject(cubeProto, cm,
                        vp, cnrm, g_renderW, g_renderH);
                    tiled.drawTextured(ct, diffuse, &ctx);
                    drawn++;
                }
            }
        }
        drawLamp(app, rz, lp);
    }
    void drawUi(Application &) override {
        ImGui::Begin("OpenGL");
        ImGui::Text("Color Picker with Alpha:");
        ImGui::ColorEdit4("Color with Alpha", m_lightColor);
        ImGui::SetNextItemWidth(200);
        int cnt = 125;
        ImGui::SliderInt("Cube Count", &m_count, 1, cnt);
        ImGui::End();
    }
    const char *name() const override { return "Spot Light Casters"; }
    const char *group() const override { return "Light"; }
private:
    int m_count{125};
};

}
