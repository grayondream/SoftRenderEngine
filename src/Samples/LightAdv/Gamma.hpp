#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>
#include <vector>

namespace SGE::Samples {

class GammaScene final : public IScene {
    float m_gammaValue{2.2f};
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
    bool m_gammaOn{true};
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
    }

    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        static Texture wood = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/wood.png");
        Rasterizer rz{fb};
        LightingRig rig{};
        rig.ambient = 0.0f;
        rig.specularStrength = m_gammaOn ? 1.0f : 1.0f;
        // reference: 5 point lights along x at y=0: red green white blue yellow
        static const ColorFlt cols[5] = {
            ColorFlt{1,0,0}, ColorFlt{0,1,0}, ColorFlt{1,1,1},
            ColorFlt{0,0,1}, ColorFlt{1,1,0}};
        for(int i = 0; i < 5; i++){
            PointLight p{};
            p.position = Vector3DBase<double>{-2.0 + i * 1.0, 0.0, 0.0};
            p.color = cols[i];
            p.range = 30.0;
            rig.point.push_back(p);
        }
        tintRig(rig);
        ShadingContext ctx{&rig, app.camera().position};
        ctx.gammaValue = m_gammaOn ? 2.2 : 1.0;
        auto fpv = refPlane(Color32{120,120,120,255});
        auto fpm = SGE::Math::translation(0.0, -0.9, -2.0);
        auto fpnrm = SGE::Math::normalMatrix(fpm);
        SGE::Render::TileRenderer tiled{fb};
        tiled.drawTextured(Pipeline::projectObject(fpv, fpm,
            refViewProj(app.camera()), fpnrm, g_renderW, g_renderH), wood, &ctx);
        for(int i = 0; i < 5; i++){
            drawLamp(app, rz,
                Vector3DBase<double>{-2.0 + i * 1.0, 0.5, 2.0},
                0.05, Color32{
                    static_cast<int32_t>(cols[i].r * 255),
                    static_cast<int32_t>(cols[i].g * 255),
                    static_cast<int32_t>(cols[i].b * 255), 255});
        }
    }
    void drawUi(Application &) override {
        ImGui::Begin("OpenGL");
        ImGui::Text("Color Picker with Alpha:");
        ImGui::ColorEdit4("Color with Alpha", m_lightColor);
        ImGui::Checkbox("Enable Gamma", &m_gammaOn);
        ImGui::InputFloat("Gamma Value", &m_gammaValue, 0.1f, 10.0f, "%.1f");
        ImGui::End();
    }
    const char *name() const override { return "Gamma Correction"; }
    const char *group() const override { return "LightAdv"; }
};

}
