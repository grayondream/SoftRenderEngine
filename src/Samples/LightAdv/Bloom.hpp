#pragma once

#include "../SceneUtil.hpp"
#include "Render/PostProcess.hpp"

#include <cmath>

namespace SGE::Samples {

class BloomScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF0A0A12u);
        LightingRig rig{};
        rig.ambient = 0.05f;
        rig.specularStrength = 0.9f;
        rig.shininess = 96.0f;
        DirectionalLight key{};
        key.direction = Vector3DBase<double>{-0.4, 0.7, -1.0};
        rig.directional.push_back(key);
        Rasterizer rz{fb};
        const auto viewProj = defaultViewProj(app);
        // bright emissive spheres
        static Object4D glow = SGE::Render::MakeSphere(0.5, 20, 14);
        for(int i = 0; i < 5; i++){
            auto gm = SGE::Math::translation(-4.0 + i * 2.0,
                1.4 + std::sin(app.angle() + i) * 0.8, 3.0);
            auto gnrm = SGE::Math::normalMatrix(gm);
            auto gt = Pipeline::projectObject(glow, gm, viewProj, gnrm, 800, 600);
            for(auto &t : gt){
                t.v[0].color = Color32{255, 240, 200, 255};
                t.v[1].color = Color32{255, 240, 200, 255};
                t.v[2].color = Color32{255, 240, 200, 255};
                rz.drawTriangleSolid(t.v[0], t.v[1], t.v[2]);
            }
        }
        if(m_bloomEnabled){
            static FrameBuffer bright{800, 600};
            SGE::Render::ExtractBright(fb, bright, m_threshold);
            SGE::Render::GaussianBlur(bright, 6);
            SGE::Render::AdditiveBlend(fb, bright);
        }
    }
    void drawUi(Application &) override {
        ImGui::Checkbox("Bloom", &m_bloomEnabled);
        ImGui::SliderFloat("Threshold", &m_threshold, 0.2f, 0.95f);
    }
    bool m_bloomEnabled{true};
    float m_threshold{0.65f};
    const char *name() const override { return "Glowing spheres with bloom"; }
    const char *group() const override { return "LightAdv"; }
};

}
