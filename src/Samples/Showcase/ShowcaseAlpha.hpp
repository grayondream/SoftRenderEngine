#pragma once

#include "../SceneUtil.hpp"
#include "Render/SortUtil.hpp"

namespace SGE::Samples {

class ShowcaseAlphaScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        auto &cam = app.camera();
        auto rig = makeDefaultRig();
        const double ang = app.angle();

        fb.clear(0xFF000000u);
        Rasterizer rz{fb};

        auto model = SGE::Math::translation(0.0, 1.6, 3.5)
            .mul(SGE::Math::rotationY(ang)).mul(SGE::Math::rotationX(0.4));
        auto nrm = SGE::Math::normalMatrix(model);
        const auto viewProj = defaultViewProj(app);

        auto torusTris = Pipeline::projectObject(app.torus(), model, viewProj, nrm, 800, 600);
        for(auto &t : torusTris){
            rz.drawTriangleSolid(t.v[0], t.v[1], t.v[2]);
        }

        struct Glass{ Point4D pos; Color32 col; };
        const Glass glass[3] = {
            {Point4D{-2.2, 0.6, 0.5, 1},  Color32{60, 160, 255, 120}},
            {Point4D{0.0, 0.9, -0.8, 1},  Color32{255, 200, 60, 130}},
            {Point4D{2.0, 0.4, 0.9, 1},   Color32{180, 80, 255, 140}}};
        const auto order = SGE::Render::SortFarToNear(3,
            [&](int i){ return glass[i].pos; }, cam.position);
        for(int idx : order){
            Object4D ball = app.sphere();
            ball.worldPos = glass[idx].pos;
            auto bm = SGE::Math::translation(glass[idx].pos.x, glass[idx].pos.y, glass[idx].pos.z);
            auto bnrm = SGE::Math::normalMatrix(bm);
            auto bt = Pipeline::projectObject(ball, bm, viewProj, bnrm, 800, 600);
            for(auto &t : bt){
                t.v[0].color = glass[idx].col;
                t.v[1].color = glass[idx].col;
                t.v[2].color = glass[idx].col;
                rz.drawTriangleSolid(t.v[0], t.v[1], t.v[2]);
            }
        }

        ScreenVertex ov[3] = {};
        ov[0] = {300, 500, -0.4f, 1};
        ov[1] = {700, 200, -0.4f, 1};
        ov[2] = {250, 150, -0.4f, 1};
        ov[0].color = Color32{40, 200, 90, 110};
        ov[1].color = Color32{40, 200, 90, 110};
        ov[2].color = Color32{40, 200, 90, 110};
        rz.drawTriangleSolid(ov[0], ov[1], ov[2]);
    }
    const char *name() const override { return "Depth + Alpha Blending"; }
    const char *group() const override { return "Showcase"; }
};

}
