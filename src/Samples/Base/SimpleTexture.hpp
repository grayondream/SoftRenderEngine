#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

namespace SGE::Samples {

class SimpleTextureScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        auto rig = makeDefaultRig();
        fb.clear(0xFF101018u);
        Rasterizer rz{fb};
        static Texture dog = SGE::Render::ImageLoader::loadTexture("assets/textures/dog.jpg");
        const double ang = app.angle();

        Object4D plane = makePlane(1.4, 0.0,
            Vector3DBase<double>{0, 0, -1}, Color32{255,255,255,255});
        auto pm = SGE::Math::translation(0.0, 0.2 * std::sin(ang), 2.5)
            .mul(SGE::Math::rotationY(ang * 0.6));
        auto pnrm = SGE::Math::normalMatrix(pm);
        ShadingContext shading{&rig, app.camera().position};
        auto pt = Pipeline::projectObject(plane, pm, defaultViewProj(app), pnrm, 800, 600);
        for(auto &t : pt){
            rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                    dog, &shading);
        }
    }
    const char *name() const override { return "Simple Texture"; }
    const char *group() const override { return "Base"; }
};

}
