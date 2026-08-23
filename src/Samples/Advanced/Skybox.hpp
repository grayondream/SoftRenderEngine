#pragma once

#include "../SceneUtil.hpp"
#include "Render/PostProcess.hpp"

#include <cmath>

namespace SGE::Samples {

class SkyboxScene final : public IScene {
public:
    void render(Application &app) override {
        static SGE::Render::HDRImage sky = []{
            return SGE::Render::ImageLoader::loadHdr("assets/textures/newport_loft.hdr");
        }();
        auto &fb = app.framebuffer();
        SGE::Render::DrawEquirectSky(fb, app.camera(), sky, m_exposure);

        LightingRig rig{};
        rig.ambient = 0.25f;
        rig.specularStrength = 0.5f;
        DirectionalLight key{};
        key.direction = Vector3DBase<double>{-0.4, 0.7, -1.0};
        rig.directional.push_back(key);
        Rasterizer rz{fb};
        ShadingContext ctx{&rig, app.camera().position};
        const auto viewProj = defaultViewProj(app);
        SGE::Render::TileRenderer tiled{fb};
        auto model = SGE::Math::translation(0.0, 1.4, 3.0)
            .mul(SGE::Math::rotationY(app.angle()))
            .mul(SGE::Math::rotationX(0.4));
        auto nrm = SGE::Math::normalMatrix(model);
        auto ct = Pipeline::projectObject(app.cube(), model, viewProj, nrm, 800, 600);
        tiled.drawTextured(ct, app.checker(), &ctx);
    }
    void drawUi(Application &) override {
        ImGui::SliderFloat("Exposure", &m_exposure, 0.2f, 3.0f);
    }
    float m_exposure{1.0f};
    const char *name() const override { return "HDR Equirect Skybox (newport loft)"; }
    const char *group() const override { return "LightAdv"; }
};

}
