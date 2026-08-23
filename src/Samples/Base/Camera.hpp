#pragma once

#include "../SceneUtil.hpp"

namespace SGE::Samples {

class CameraWalkScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        auto rig = makeDefaultRig();
        auto fog = defaultFog(app);
        fb.clear(0xFF101018u);
        Rasterizer rz{fb};
        ShadingContext shading{&rig, app.camera().position,
            app.fogEnabled() ? &fog : nullptr};
        SGE::Render::TileRenderer tiled{fb};
        const auto viewProj = defaultViewProj(app);
        const auto nrm = SGE::Math::normalMatrix(
            SGE::Math::translation(0.0, 0.0, 0.0));

        Object4D ground = makePlane(12.0, -2.0,
            Vector3DBase<double>{0, 1, 0}, Color32{200, 200, 205, 255}, 24.0);
        auto gt = Pipeline::projectObject(ground,
            SGE::Math::translation(0.0,0.0,0.0), viewProj, nrm, 800, 600);
        tiled.drawTextured(gt, app.checker(), &shading);

        for(int i = 0; i < 8; i++){
            Object4D cube = app.cube();
            const double a = i * M_PI / 4;
            const double cx = std::cos(a) * 5.0;
            const double cz = std::sin(a) * 5.0 + 3.0;
            auto cm = SGE::Math::translation(cx, -1.0, cz)
                .mul(SGE::Math::rotationY(app.angle() * (i % 2 ? -1 : 1)));
            auto cnrm = SGE::Math::normalMatrix(cm);
            auto ct = Pipeline::projectObject(cube, cm, viewProj, cnrm, 800, 600);
            tiled.drawTextured(ct, app.checker(), &shading);
        }
    }
    void drawUi(Application &) override {
        ImGui::Text("WASD/RF move, arrows look");
    }
    const char *name() const override { return "Camera Walkthrough"; }
    const char *group() const override { return "Base"; }
};

}
