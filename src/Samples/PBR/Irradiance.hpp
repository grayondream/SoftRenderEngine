#pragma once

#include "../SceneUtil.hpp"
#include "Render/PostProcess.hpp"

#include <cmath>

namespace SGE::Samples {

class IrradianceScene final : public IScene {
public:
    void render(Application &app) override {
        static SGE::Render::HDRImage env = []{
            return SGE::Render::ImageLoader::loadHdr("assets/textures/newport_loft.hdr");
        }();
        auto &fb = app.framebuffer();
        SGE::Render::DrawEquirectSky(fb, app.camera(), env, m_exposure);
        LightingRig rig{};
        rig.ambient = 0.05f;
        rig.specularStrength = 0.1f;
        DirectionalLight key{};
        key.direction = Vector3DBase<double>{-0.3, 0.8, -0.9};
        key.color = ColorFlt{0.35f, 0.35f, 0.35f};
        rig.directional.push_back(key);

        Rasterizer rz{fb};
        ShadingContext ctx{&rig, app.camera().position};
        ctx.iblEquirect = &env;
        const auto viewProj = defaultViewProj(app);
        // row of white spheres lit by the environment
        for(int i = 0; i < 5; i++){
            Object4D sphere = SGE::Render::MakeSphere(0.55, 24, 16);
            Texture white(1, 1, std::vector<uint32_t>{0xFFF0F0F0u}.data());
            auto sm = SGE::Math::translation(-3.2 + i * 1.6, 1.1, 3.0);
            auto snrm = SGE::Math::normalMatrix(sm);
            auto st = Pipeline::projectObject(sphere, sm, viewProj, snrm, 800, 600);
            SGE::Render::TileRenderer tiled{fb};
            tiled.drawTextured(st, white, &ctx);
        }
    }
    void drawUi(Application &) override {
        ImGui::SliderFloat("Exposure", &m_exposure, 0.3f, 2.5f);
        ImGui::Text("Diffuse IBL from equirect env");
    }
    float m_exposure{1.0f};
    const char *name() const override { return "IBL Diffuse Irradiance (loft)"; }
    const char *group() const override { return "PBR"; }
};

}
