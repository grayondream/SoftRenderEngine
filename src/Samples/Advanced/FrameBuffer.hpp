#pragma once

#include "../SceneUtil.hpp"
#include "Render/PostProcess.hpp"

#include <cmath>

namespace SGE::Samples {

class FrameBufferScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        // render a mini-scene off-screen, then use it as texture on a cube
        static FrameBuffer mini{256, 256};
        static Texture miniTex{};
        LightingRig rig{};
        rig.ambient = 0.25f;
        rig.specularStrength = 0.6f;
        DirectionalLight key{};
        key.direction = Vector3DBase<double>{-0.5, 0.7, -1.0};
        rig.directional.push_back(key);
        ShadingContext ctx{&rig, app.camera().position};
        mini.clear(0xFF203048u);
        {
            Rasterizer mrz{mini};
            SGE::Render::Camera mc{};
            mc.position = Vector3DBase<double>{0, 1.5, -3.5};
            mc.yaw = 0.0; mc.pitch = -0.35;
            auto view = mc.viewMatrix();
            auto proj = SGE::Math::perspective(M_PI / 3, 1.0, 0.1, 50.0);
            auto vp = proj.mul(view);
            Object4D sphere = SGE::Render::MakeSphere(0.9, 22, 15);
            auto sm = SGE::Math::translation(0.0, 0.9, 0.0)
                .mul(SGE::Math::rotationY(app.angle() * 1.5));
            auto snrm = SGE::Math::normalMatrix(sm);
            auto st = Pipeline::projectObject(sphere, sm, vp, snrm, 256, 256);
            SGE::Render::TileRenderer tiled{mini};
            tiled.drawTextured(st, app.checker(), &ctx);
        }
        // refresh texture from framebuffer
        miniTex = Texture(mini.width(), mini.height(), mini.colorData());
        miniTex.buildMipChain();

        const auto viewProj = defaultViewProj(app);
        Rasterizer rz{fb};
        auto model = SGE::Math::translation(0.0, 1.5, 3.2)
            .mul(SGE::Math::rotationY(app.angle()))
            .mul(SGE::Math::rotationX(0.45));
        auto nrm = SGE::Math::normalMatrix(model);
        auto ct = Pipeline::projectObject(app.cube(), model, viewProj, nrm, 800, 600);
        for(auto &t : ct){
            rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                    miniTex, nullptr);
        }
    }
    const char *name() const override { return "Render-to-Texture Cube"; }
    const char *group() const override { return "Advanced"; }
};

}
