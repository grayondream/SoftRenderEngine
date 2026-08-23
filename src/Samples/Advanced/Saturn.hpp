#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>

namespace SGE::Samples {

class SaturnScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF06060Cu);
        LightingRig rig{};
        rig.ambient = 0.10f;
        rig.specularStrength = 0.5f;
        DirectionalLight sun{};
        sun.direction = Vector3DBase<double>{-0.6, 0.4, -0.8};
        rig.directional.push_back(sun);
        Rasterizer rz{fb};
        const auto viewProj = defaultViewProj(app);
        SGE::Render::TileRenderer tiled{fb};
        ShadingContext ctx{&rig, app.camera().position};

        Object4D planet = SGE::Render::MakeSphere(1.3, 28, 18);
        auto pm = SGE::Math::translation(0.0, 1.2, 3.0)
            .mul(SGE::Math::rotationY(app.angle() * 0.8));
        auto pnrm = SGE::Math::normalMatrix(pm);
        auto pt = Pipeline::projectObject(planet, pm, viewProj, pnrm, 800, 600);
        tiled.drawTextured(pt, app.checker(), &ctx);

        // ring of orbiting cubes
        static Object4D rock = SGE::Render::MakeSphere(0.09, 10, 7);
        const double tilt = 0.42;
        for(int i = 0; i < 64; i++){
            const double th = app.angle() * 1.4 + i * M_PI / 32;
            const double rr = (i % 2 == 0) ? 2.05 : 2.45;
            double lx = std::cos(th) * rr;
            double lz = std::sin(th) * rr;
            double ly = -lz * tilt;
            lz = lz * std::cos(tilt);
            auto rm = SGE::Math::translation(lx, 1.2 + ly, 3.0 + lz);
            auto rnrm = SGE::Math::normalMatrix(rm);
            auto rt = Pipeline::projectObject(rock, rm, viewProj, rnrm, 800, 600);
            tiled.drawTextured(rt, app.checker(), &ctx);
        }
    }
    void drawUi(Application &) override {
        ImGui::Text("Planet with tilted debris ring");
    }
    const char *name() const override { return "Saturn Ring System"; }
    const char *group() const override { return "Advanced"; }
};

}
