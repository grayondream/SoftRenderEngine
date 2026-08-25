#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>

namespace SGE::Samples {

class BlinnPhongScene final : public IScene {
public:
    bool m_blinn{true};
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        static Texture wood = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/wood.png");
        Rasterizer rz{fb};
        // reference: point light at origin, lamp 0.05, amb .05 spec .3
        LightingRig rig{};
        rig.ambient = 0.05f;
        rig.specularStrength = 0.3f;
        rig.shininess = m_blinn ? 32.0f : 8.0f;
        PointLight p{};
        p.position = Vector3DBase<double>{0, 0, 2.0};
        p.range = 60.0;
        rig.point.push_back(p);
        ShadingContext ctx{&rig, app.camera().position};
        auto fpv = refPlane(Color32{160,160,160,255}, 4.0);
        auto fpm = SGE::Math::translation(0.0, -0.9, -2.0);
        auto fpnrm = SGE::Math::normalMatrix(fpm);
        SGE::Render::TileRenderer tiled{fb};
        tiled.drawTextured(Pipeline::projectObject(fpv, fpm,
            refViewProj(app.camera()), fpnrm, g_renderW, g_renderH), wood, &ctx);
        drawLamp(app, rz, Vector3DBase<double>{0, 0, 2.0}, 0.06);
    }
    void drawUi(Application &) override {
        ImGui::Checkbox("Blinn-Phong (shininess 32)", &m_blinn);
        if(!m_blinn){
            ImGui::Text("Phong model (shininess 8)");
        }
    }
    const char *name() const override { return "Blinn-Phong vs Phong"; }
    const char *group() const override { return "LightAdv"; }
};

}
