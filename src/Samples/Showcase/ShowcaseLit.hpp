#pragma once

#include "../SceneUtil.hpp"
#include "Render/SortUtil.hpp"

#include <cstdio>
#include <algorithm>

namespace SGE::Samples {

class ShowcaseLitScene final : public IScene {
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

        Object4D sphere = app.sphere();
        Object4D torus = app.torus();
        Object4D teapot = app.teapot();
        const auto rot = SGE::Math::rotationY(ang * 0.5);
        struct Item{ const Object4D *obj; double ox, oy, oz; };
        const Item items[] = {
            {&sphere, sphere.worldPos.x, sphere.worldPos.y + 0.8, sphere.worldPos.z},
            {&torus,  torus.worldPos.x,  torus.worldPos.y + 1.0,  torus.worldPos.z},
            {&teapot, teapot.worldPos.x, teapot.worldPos.y + 0.75, teapot.worldPos.z}};
        SGE::Render::TileRenderer tiler{fb};
        const auto viewProj = defaultViewProj(app);
        for(const auto &it : items){
            auto im = SGE::Math::translation(it.ox, it.oy, it.oz).mul(rot);
            auto inrm = SGE::Math::normalMatrix(im);
            auto t2 = Pipeline::projectObject(*it.obj, im, viewProj, inrm, 800, 600);
            tiler.drawTextured(t2, app.checker(), &shading);
        }
        auto model = SGE::Math::translation(0.0, 1.6, 3.5)
            .mul(SGE::Math::rotationY(ang)).mul(SGE::Math::rotationX(0.4));
        auto nrm = SGE::Math::normalMatrix(model);
        auto ct = Pipeline::projectObject(app.cube(), model, viewProj, nrm, 800, 600);
        tiler.drawTextured(ct, app.checker(), &shading);
    }
    const char *name() const override { return "Geometry Showcase + Lighting"; }
    const char *group() const override { return "Showcase"; }
};

}
