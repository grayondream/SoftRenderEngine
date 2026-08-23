#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

namespace SGE::Samples {

class RectScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        auto rig = makeDefaultRig();
        fb.clear(0xFF101018u);
        Rasterizer rz{fb};
        static Texture wood = SGE::Render::ImageLoader::loadTexture("assets/textures/wood.png");
        Object4D plane = makePlane(1.0, 0.0,
            Vector3DBase<double>{0, 0, -1}, Color32{255,255,255,255});
        auto pm = SGE::Math::translation(0.0, 0.0, 2.0);
        auto pnrm = SGE::Math::normalMatrix(pm);
        auto pt = Pipeline::projectObject(plane, pm, defaultViewProj(app), pnrm, 800, 600);
        for(auto &t : pt){
            rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                    wood, nullptr, TextureFilter::Bilinear,
                                    TextureWrap::Clamp);
        }
    }
    const char *name() const override { return "Textured Rect"; }
    const char *group() const override { return "Base"; }
};

}
