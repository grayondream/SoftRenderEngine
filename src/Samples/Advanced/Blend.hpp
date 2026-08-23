#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"
#include "Light/LightUtil.hpp"

#include <cmath>

namespace SGE::Samples {

class BlendScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        static Texture windowTex = SGE::Render::ImageLoader::loadTexture("assets/textures/window.png");
        LightingRig rig{};
        rig.ambient = 0.5f;
        rig.directional.clear();
        const double a = app.angle();
        Rasterizer rz{fb};
        // opaque cube first
        const auto viewProj = defaultViewProj(app);
        {
            auto cm = SGE::Math::translation(0.0, 1.4, 2.0);
            auto cnrm = SGE::Math::normalMatrix(cm);
            ShadingContext ctx{&rig, app.camera().position};
            SGE::Render::TileRenderer tiled{fb};
            auto ct = Pipeline::projectObject(app.cube(), cm, viewProj, cnrm, 800, 600);
            tiled.drawTextured(ct, app.checker(), &ctx);
        }
        // sorted transparent quads
        struct Win{ Point4D pos; };
        std::vector<Win> wins;
        for(int i = 0; i < 6; i++){
            wins.push_back(Win{Point4D{-2.8 + i * 1.15, 1.4,
                2.2 - i * 0.35 + 1.4 * std::sin(a), 1}});
        }
        const auto order = SGE::Render::SortFarToNear(
            static_cast<int>(wins.size()),
            [&](int i){ return wins[i].pos; }, app.camera().position);
        Object4D quad = makePlane(0.55, 0.0,
            Vector3DBase<double>{0, 0, -1}, Color32{255,255,255,255});
        for(int idx : order){
            auto wm = SGE::Math::translation(wins[idx].pos.x, wins[idx].pos.y,
                wins[idx].pos.z).mul(SGE::Math::rotationY(std::sin(a) * 0.3));
            auto wnrm = SGE::Math::normalMatrix(wm);
            auto wt = Pipeline::projectObject(quad, wm, viewProj, wnrm, 800, 600);
            for(auto &t : wt){
                rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                        windowTex, nullptr);
            }
        }
    }
    const char *name() const override { return "Alpha Blend Windows"; }
    const char *group() const override { return "Advanced"; }
};

}
