#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

namespace SGE::Samples {

class CubeTextureScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        auto rig = makeDefaultRig();
        fb.clear(0xFF101018u);
        Rasterizer rz{fb};
        static Texture container = SGE::Render::ImageLoader::loadTexture("assets/textures/container2.jpg");
        const double ang = app.angle();

        auto model = SGE::Math::translation(0.0, 1.6, 3.5)
            .mul(SGE::Math::rotationY(ang))
            .mul(SGE::Math::rotationX(0.4));
        auto nrm = SGE::Math::normalMatrix(model);
        ShadingContext shading{&rig, app.camera().position};
        auto tris = Pipeline::projectObject(app.cube(), model,
            defaultViewProj(app), nrm, 800, 600);
        for(auto &t : tris){
            rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                    container, &shading);
        }
    }
    const char *name() const override { return "Textured Cube (container2)"; }
    const char *group() const override { return "Base"; }
};

}
