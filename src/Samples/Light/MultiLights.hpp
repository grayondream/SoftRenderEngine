#pragma once

#include "../SceneUtil.hpp"
#include "LightUtil.hpp"

#include <cmath>
#include <vector>

namespace SGE::Samples {

class MultiLightsScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        LightingRig rig{};
        rig.ambient = 0.03f;
        rig.specularStrength = 0.55f;
        const double a = app.angle();
        const ColorFlt cols[4] = {
            ColorFlt{1.0f, 0.35f, 0.30f}, ColorFlt{0.35f, 1.0f, 0.40f},
            ColorFlt{0.35f, 0.50f, 1.00f}, ColorFlt{1.00f, 0.90f, 0.30f}};
        std::vector<Vector3DBase<double>> lpos;
        for(int i = 0; i < 4; i++){
            const double th = a + i * M_PI / 2;
            PointLight p{};
            p.position = Vector3DBase<double>{
                3.2 * std::sin(th), 2.6, 2.0 + 3.2 * std::cos(th)};
            p.color = cols[i];
            p.range = 16.0;
            rig.point.push_back(p);
            lpos.push_back(p.position);
        }
        Rasterizer rz{fb};
        drawLitCubeWall(app, rig, rz, app.angle());
        for(int i = 0; i < 4; i++){
            const auto &c = cols[i];
            drawLightMarker(app, rz, lpos[i], Color32{
                static_cast<int32_t>(c.r * 255),
                static_cast<int32_t>(c.g * 255),
                static_cast<int32_t>(c.b * 255), 255});
        }
    }
    void drawUi(Application &) override {
        ImGui::Text("4 colored point lights orbiting");
    }
    const char *name() const override { return "Multiple Lights"; }
    const char *group() const override { return "Light"; }
};

}
