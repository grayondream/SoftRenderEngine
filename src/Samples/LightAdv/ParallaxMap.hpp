#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"
#include "Light/LightUtil.hpp"

#include <cmath>

namespace SGE::Samples {

class ParallaxMapScene final : public IScene {
public:
    bool m_enableDisp{true};
    bool m_enableSteep{true};
    bool m_enableOcclusion{false};
    float m_heightScale{0.1f};
    void setup(Application &app) override {
        resetCamera(app, 0.0, 1.4, 3.0);
    }

    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        static Texture diffuse = SGE::Render::ImageLoader::loadTexture("assets/textures/bricks2.jpg");
        static Texture normal = SGE::Render::ImageLoader::loadTexture("assets/textures/bricks2_normal.jpg");
        static Texture height = SGE::Render::ImageLoader::loadTexture("assets/textures/bricks2_disp.jpg");
        LightingRig rig{};
        rig.ambient = 0.1f;
        rig.specularStrength = 0.2f;
        const double a = app.angle();
        PointLight p{};
        p.position = Vector3DBase<double>{1.8 * std::sin(a * 1.3), 2.2,
            -1.0 + 1.8 * std::cos(a * 1.3)};
        p.range = 20.0;
        rig.point.push_back(p);

        Rasterizer rz{fb};
        const auto viewProj = defaultViewProj(app);
        ShadingContext ctx{&rig, app.camera().position};
        ctx.normalTex = m_enableDisp ? &normal : nullptr;
        ctx.heightTex = (m_enableDisp && m_enableSteep) ? &height : nullptr;
        ctx.parallaxScale = static_cast<double>(m_heightScale);
        ctx.tangentU = Vector3DBase<double>{1, 0, 0};
        ctx.tangentV = Vector3DBase<double>{0, 1, 0};
        Object4D wall = makePlane(2.6, 0.0,
            Vector3DBase<double>{0, 0, -1}, Color32{255,255,255,255}, 5.0);
        auto wm = SGE::Math::translation(0.0, 1.4, 2.2)
            .mul(SGE::Math::rotationY(std::sin(a) * 0.35));
        auto wnrm = SGE::Math::normalMatrix(wm);
        SGE::Render::TileRenderer tiled{fb};
        auto wt = Pipeline::projectObject(wall, wm, viewProj, wnrm, g_renderW, g_renderH);
        tiled.drawTextured(wt, diffuse, &ctx);
        drawLightMarker(app, rz, p.position, Color32{255, 250, 220, 255});
    }
    void drawUi(Application &) override {
        ImGui::Begin("OpenGL");
        ImGui::Checkbox("Enable Normal Map", &m_enableDisp);
        ImGui::Checkbox("Enable Steep", &m_enableSteep);
        ImGui::Checkbox("Enable Occlusion", &m_enableOcclusion);
        ImGui::InputFloat("Height Scale", &m_heightScale, 0.1f, 1.0f, "%.2f");
        ImGui::End();
    }
    const char *name() const override { return "Parallax Mapping (steep)"; }
    const char *group() const override { return "LightAdv"; }
};

}
