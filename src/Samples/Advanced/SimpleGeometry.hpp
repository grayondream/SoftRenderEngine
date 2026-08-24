#pragma once

#include "../SceneUtil.hpp"

#include <cmath>

namespace SGE::Samples {

// GL AppType: SimpleGeometry — primitive generator showcase
class SimpleGeometryScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        auto rig = makeDefaultRig();
        auto fog = defaultFog(app);
        fb.clear(0xFF101018u);
        ShadingContext shading{&rig, app.camera().position,
            app.fogEnabled() ? &fog : nullptr};
        const double ang = app.angle();
        const auto viewProj = defaultViewProj(app);
        SGE::Render::TileRenderer tiled{fb};

        Object4D sphere = SGE::Render::MakeSphere(0.85, 24, 16);
        auto m1 = SGE::Math::translation(-3.6, 0.6, 2.5).mul(SGE::Math::rotationY(ang * 0.7));
        auto n1 = SGE::Math::normalMatrix(m1);
        tiled.drawTextured(Pipeline::projectObject(sphere, m1, viewProj, n1, 800, 600),
                           app.checker(), &shading);

        Object4D torus = SGE::Render::MakeTorus(0.95, 0.35, 32, 16);
        auto m2 = SGE::Math::translation(-1.2, 0.6, 3.0).mul(SGE::Math::rotationY(ang));
        auto n2 = SGE::Math::normalMatrix(m2);
        tiled.drawTextured(Pipeline::projectObject(torus, m2, viewProj, n2, 800, 600),
                           app.checker(), &shading);

        Object4D teapot = SGE::Render::MakeTeapot();
        teapot.worldPos = Point4D{0, -1.55, 2.2, 1};
        auto m3 = SGE::Math::translation(1.4, -0.35, 3.2).mul(SGE::Math::rotationY(ang * 0.5));
        auto n3 = SGE::Math::normalMatrix(m3);
        tiled.drawTextured(Pipeline::projectObject(teapot, m3, viewProj, n3, 800, 600),
                           app.checker(), &shading);

        Object4D cone = SGE::Render::MakeCone(0.75, 1.9);
        auto m4 = SGE::Math::translation(3.8, 0.15, 2.8).mul(SGE::Math::rotationY(ang * 1.3));
        auto n4 = SGE::Math::normalMatrix(m4);
        tiled.drawTextured(Pipeline::projectObject(cone, m4, viewProj, n4, 800, 600),
                           app.checker(), &shading);

        Object4D cylinder = SGE::Render::MakeCylinder(0.55, 1.8, 24);
        auto m5 = SGE::Math::translation(0.1, 0.45, 1.2).mul(SGE::Math::rotationX(0.25))
            .mul(SGE::Math::rotationY(ang * 0.9));
        auto n5 = SGE::Math::normalMatrix(m5);
        tiled.drawTextured(Pipeline::projectObject(cylinder, m5, viewProj, n5, 800, 600),
                           app.checker(), &shading);
    }
    void drawUi(Application &) override {
        ImGui::Text("sphere / torus / utah teapot / cone / cylinder");
    }
    const char *name() const override { return "Simple Geometry"; }
    const char *group() const override { return "Advanced"; }
};

}
