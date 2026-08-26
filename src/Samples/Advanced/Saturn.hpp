#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>
#include <vector>

namespace SGE::Samples {

class SaturnScene final : public IScene {
public:
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        LightingRig rig{};
        rig.ambient = 0.35f;
        rig.specularStrength = 0.3f;
        DirectionalLight key{};
        key.direction = Vector3DBase<double>{-0.5, 0.6, -1.0};
        rig.directional.push_back(key);
        Rasterizer rz{fb};
        ShadingContext ctx{&rig, app.camera().position};
        const auto vp = refViewProj(app.camera());
        // planet at origin-ish center (reference T(0,0,-3) S0.3 of big mesh)
        static Object4D planet = SGE::Render::MakeSphere(1.0, 28, 18);
        Texture tint(1, 1, std::vector<uint32_t>{0xFFC8A05A}.data());
        const double spin = app.angle() * 0.6;
        auto pm = SGE::Math::translation(0.0, 0.0, -3.0 + 3.0)
            .mul(SGE::Math::translation(0.0, 0.9, 0.0))
            .mul(SGE::Math::rotationY(spin));
        auto pnrm = SGE::Math::normalMatrix(pm);
        SGE::Render::TileRenderer tiled{fb};
        tiled.drawTextured(Pipeline::projectObject(planet, pm,
            vp, pnrm, g_renderW, g_renderH), tint, &ctx);
        // rock ring: radius 20->scaled 4, y*0.4, slow orbit
        static Object4D rock = SGE::Render::MakeSphere(1.0, 8, 5);
        constexpr int kRocks = 240;
        for(int i = 0; i < kRocks; i++){
            const double angle = i * 360.0 / kRocks;
            const double disp = ((i * 37) % 200) / 10.0 - 10.0;
            const double rad = angle * M_PI / 180.0;
            const double rr = 4.0;
            double x = std::sin(rad) * rr + disp * 0.08;
            double y = disp * 0.032;
            double z = std::cos(rad) * rr + disp * 0.08;
            const double sc = ((i * 13) % 20) / 100.0 + 0.05;
            const double orbit = app.angle() / 10.0;
            const double cx = std::cos(orbit), sy2 = std::sin(orbit);
            const double wx = x * cx - z * sy2;
            const double wz = x * sy2 + z * cx;
            auto rm = SGE::Math::translation(wx, y + 0.9, wz - 0.0)
                .mul(SGE::Math::scale(sc, sc, sc));
            auto rnrm = SGE::Math::normalMatrix(rm);
            tiled.drawTextured(Pipeline::projectObject(rock, rm,
                vp, rnrm, g_renderW, g_renderH), tint, &ctx);
        }
    }
    void drawUi(Application &) override {
        ImGui::Begin("OpenGL");
        ImGui::End();
    }
    const char *name() const override { return "Saturn Ring System"; }
    const char *group() const override { return "Advanced"; }
};

}
