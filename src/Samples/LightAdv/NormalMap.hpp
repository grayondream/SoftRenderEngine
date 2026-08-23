#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"
#include "Light/LightUtil.hpp"

#include <cmath>

namespace SGE::Samples {

class NormalMapScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        static Texture diffuse = SGE::Render::ImageLoader::loadTexture("assets/textures/brickwall.jpg");
        static Texture normal = SGE::Render::ImageLoader::loadTexture("assets/textures/brickwall_normal.jpg");
        LightingRig rig{};
        rig.ambient = 0.15f;
        rig.specularStrength = 0.25f;
        const double a = app.angle();
        PointLight p{};
        p.position = Vector3DBase<double>{2.5 * std::sin(a), 2.5, -1.0 + 2.5 * std::cos(a)};
        p.range = 20.0;
        rig.point.push_back(p);

        Rasterizer rz{fb};
        const auto viewProj = defaultViewProj(app);
        // wall facing camera: N=(0,0,-1) T=(1,0,0) B=(0,1,0)
        ShadingContext ctx{&rig, app.camera().position};
        ctx.normalTex = &normal;
        ctx.tangentU = Vector3DBase<double>{1, 0, 0};
        ctx.tangentV = Vector3DBase<double>{0, 1, 0};
        Object4D wall = makePlane(2.6, 0.0,
            Vector3DBase<double>{0, 0, -1}, Color32{255,255,255,255}, 4.0);
        auto wm = SGE::Math::translation(0.0, 1.4, 2.2);
        auto wnrm = SGE::Math::normalMatrix(wm);
        SGE::Render::TileRenderer tiled{fb};
        auto wt = Pipeline::projectObject(wall, wm, viewProj, wnrm, 800, 600);
        tiled.drawTextured(wt, diffuse, &ctx);
        drawLightMarker(app, rz, p.position, Color32{255, 250, 220, 255});
    }
    const char *name() const override { return "Normal Mapping (brickwall)"; }
    const char *group() const override { return "LightAdv"; }
};

}
