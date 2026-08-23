#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"
#include "Light/LightUtil.hpp"

#include <cmath>

namespace SGE::Samples {

class CullFaceScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        LightingRig rig{};
        rig.ambient = 0.35f;
        const double a = app.angle();
        Rasterizer rz{fb};
        rz.cullBackface = m_cullEnabled;
        const auto viewProj = defaultViewProj(app);
        for(int i = 0; i < 3; i++){
            Object4D cube = app.cube();
            auto cm = SGE::Math::translation(-2.5 + i * 2.5, 1.4, 2.5)
                .mul(SGE::Math::rotationY(a)).mul(SGE::Math::rotationX(0.4));
            auto cnrm = SGE::Math::normalMatrix(cm);
            auto ct = Pipeline::projectObject(cube, cm, viewProj, cnrm, 800, 600);
            for(auto &t : ct){
                const int32_t shade = 80 + i * 60;
            t.v[0].color = Color32{shade, 160, 220, 255};
                t.v[1].color = Color32{shade, 160, 220, 255};
                t.v[2].color = Color32{shade, 160, 220, 255};
                rz.drawTriangleSolid(t.v[0], t.v[1], t.v[2]);
            }
        }
    }
    void drawUi(Application &app) override {
        (void)app;
        ImGui::Checkbox("Cull Backface", &m_cullEnabled);
    }
    bool m_cullEnabled{true};
    const char *name() const override { return "Backface Culling"; }
    const char *group() const override { return "Advanced"; }
};

}
