#pragma once

#include "../SceneUtil.hpp"
#include "Render/PostProcess.hpp"

#include <cmath>

namespace SGE::Samples {

class SSAOScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        LightingRig rig{};
        rig.ambient = 0.35f;
        rig.specularStrength = 0.2f;
        DirectionalLight key{};
        key.direction = Vector3DBase<double>{-0.5, 0.7, -1.0};
        rig.directional.push_back(key);
        ShadingContext ctx{&rig, app.camera().position};
        const auto viewProj = defaultViewProj(app);
        SGE::Render::TileRenderer tiled{fb};
        for(int i = 0; i < 4; i++){
            Object4D cube = app.cube();
            auto cm = SGE::Math::translation(-3.0 + i * 2.0, 0.9, 2.5)
                .mul(SGE::Math::rotationY(app.angle() * 0.4 + i));
            auto cnrm = SGE::Math::normalMatrix(cm);
            auto ct = Pipeline::projectObject(cube, cm, viewProj, cnrm, 800, 600);
            tiled.drawTextured(ct, app.checker(), &ctx);
        }
        if(m_ssaoOn){
            // darken pixels whose neighborhood depth varies strongly
            static std::vector<uint32_t> src(
                fb.width() * fb.height());
            std::copy(fb.colorData(),
                fb.colorData() + src.size(), src.begin());
            static std::vector<float> dep(fb.width() * fb.height());
            std::copy(fb.depthData(),
                fb.depthData() + dep.size(), dep.begin());
            const std::size_t w = fb.width(), h = fb.height();
            for(std::size_t y = 1; y + 1 < h; y++){
                for(std::size_t x = 1; x + 1 < w; x++){
                    const float d = dep[y * w + x];
                    if(d <= 0.0f || d >= 1.0f){ continue; }
                    float occ = 0.0f;
                    for(int oy = -1; oy <= 1; oy++){
                        for(int ox = -1; ox <= 1; ox++){
                            const float dn =
                                dep[(y + oy) * w + (x + ox)];
                            if(dn > d + 0.008f){ occ += 0.14f; }
                        }
                    }
                    occ = std::min(occ, 0.65f);
                    if(occ <= 0.01f){ continue; }
                    const uint32_t c = src[y * w + x];
                    const uint32_t nr = static_cast<uint32_t>(
                        ((c >> 16) & 0xFF) * (1.0f - occ));
                    const uint32_t ng = static_cast<uint32_t>(
                        ((c >> 8) & 0xFF) * (1.0f - occ));
                    const uint32_t nb = static_cast<uint32_t>(
                        (c & 0xFF) * (1.0f - occ));
                    fb.setPixel(x, y,
                        0xFF000000u | (nr << 16) | (ng << 8) | nb, -2.0f);
                }
            }
        }
    }
    void drawUi(Application &) override {
        ImGui::Checkbox("SSAO", &m_ssaoOn);
    }
    bool m_ssaoOn{true};
    const char *name() const override { return "SSAO (depth contrast)"; }
    const char *group() const override { return "LightAdv"; }
};

}
