#pragma once

#include "../SceneUtil.hpp"

#include <cstdio>

namespace SGE::Samples {

class ShowcaseFilterScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        auto &cam = app.camera();
        auto rig = makeDefaultRig();
        auto fog = defaultFog(app);
        ShadingContext shading{&rig, cam.position, app.fogEnabled() ? &fog : nullptr};
        const double ang = app.angle();

        fb.clear(0xFF000000u);
        Rasterizer rz{fb};

        auto checkerBig = app.checker();
        checkerBig.buildMipChain();
        SGE::Render::TileRenderer tiled{fb};
        const double spread[4] = {1.0, 2.2, 4.6, 9.0};
        const auto viewProj = defaultViewProj(app);
        for(int i = 0; i < 4; i++){
            Object4D plane = makePlane(0.9, 0.0,
                Vector3DBase<double>{0, 0, -1}, Color32{255,255,255,255}, 12.0);
            auto pm = SGE::Math::translation(
                -3.5 + i * 2.35, 0.0,
                -(spread[i] * 0.35) + spread[i]).mul(SGE::Math::rotationY(ang * 0.3));
            auto pnrm = SGE::Math::normalMatrix(pm);
            auto pt = Pipeline::projectObject(plane, pm, viewProj, pnrm, 800, 600);
            TextureFilter f = (i % 2 == 0)
                ? TextureFilter::Nearest
                : TextureFilter::Trilinear;
            for(auto &t : pt){
                rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                        checkerBig, &shading, f, TextureWrap::Repeat);
            }
        }
    }
    const char *name() const override { return "Mipmap + Trilinear vs Nearest"; }
    const char *group() const override { return "Showcase"; }
};

}
