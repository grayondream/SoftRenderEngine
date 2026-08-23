#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"
#include "Light/LightUtil.hpp"

#include <cmath>

namespace SGE::Samples {

class MultiInstanceScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        auto fog = defaultFog(app);
        fb.clear(0xFF101018u);
        LightingRig rig{};
        rig.ambient = 0.18f;
        rig.specularStrength = 0.3f;
        DirectionalLight key{};
        key.direction = Vector3DBase<double>{-0.5, 0.7, -1.0};
        rig.directional.push_back(key);
        Rasterizer rz{fb};
        ShadingContext ctx{&rig, app.camera().position,
            app.fogEnabled() ? &fog : nullptr};
        const auto viewProj = defaultViewProj(app);
        const int grid = 10;
        int drawn = 0;
        SGE::Render::TileRenderer tiled{fb};
        for(int gx = 0; gx < grid; gx++){
            for(int gz = 0; gz < grid; gz++){
                Object4D cube = app.cube();
                const double cx = (gx - grid / 2.0 + 0.5) * 1.1;
                const double cz = 2.5 + (gz - grid / 2.0 + 0.5) * 1.1;
                auto cm = SGE::Math::translation(cx, -1.6, cz)
                    .mul(SGE::Math::rotationY(app.angle() + gx + gz));
                auto cnrm = SGE::Math::normalMatrix(cm);
                auto ct = Pipeline::projectObject(cube, cm, viewProj, cnrm, 800, 600);
                tiled.drawTextured(ct, app.checker(), &ctx);
                drawn++;
            }
        }
        m_count = drawn;
    }
    void drawUi(Application &) override {
        ImGui::Text("%d cubes reusing one mesh", m_count);
    }
    int m_count{0};
    const char *name() const override { return "100 Cubes Instancing"; }
    const char *group() const override { return "Advanced"; }
};

}
