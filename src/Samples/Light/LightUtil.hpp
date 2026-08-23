#pragma once

#include "../SceneUtil.hpp"

namespace SGE::Samples {

// Shared: lit cube wall used by light-source demos
inline void drawLitCubeWall(Application &app, const LightingRig &rig,
    [[maybe_unused]] Rasterizer &rz, double angle){
    const auto viewProj = defaultViewProj(app);
    for(int r = 0; r < 2; r++){
        for(int c = 0; c < 4; c++){
            Object4D cube = app.cube();
            const double cx = -3.0 + c * 2.0;
            const double cy = -0.5 + r * 2.2;
            auto cm = SGE::Math::translation(cx, cy, 2.5)
                .mul(SGE::Math::rotationY(angle * (r + c + 1) * 0.25))
                .mul(SGE::Math::rotationX(0.35));
            auto cnrm = SGE::Math::normalMatrix(cm);
            ShadingContext ctx{&rig, app.camera().position};
            auto ct = Pipeline::projectObject(cube, cm, viewProj, cnrm, 800, 600);
            SGE::Render::TileRenderer tiled{app.framebuffer()};
            tiled.drawTextured(ct, app.checker(), &ctx);
        }
    }
}

inline void drawLightMarker(Application &app, Rasterizer &rz,
    const Vector3DBase<double> &pos, const Color32 &color){
    static Object4D marker = []{ return SGE::Render::MakeSphere(0.18, 12, 8); }();
    auto mm = SGE::Math::translation(pos.x, pos.y, pos.z);
    auto mnrm = SGE::Math::normalMatrix(mm);
    auto mt = Pipeline::projectObject(marker, mm,
        defaultViewProj(app), mnrm, 800, 600);
    for(auto &t : mt){
        t.v[0].color = color; t.v[1].color = color; t.v[2].color = color;
        rz.drawTriangleSolid(t.v[0], t.v[1], t.v[2]);
    }
}

}
