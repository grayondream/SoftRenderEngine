#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>

namespace SGE::Samples {

class LightMapScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        static Texture diffuse = SGE::Render::ImageLoader::loadTexture("assets/textures/container2.jpg");
        LightingRig rig{};
        rig.ambient = 0.12f;
        rig.specularStrength = 0.9f;
        rig.shininess = 48.0f;
        DirectionalLight key{};
        key.direction = Vector3DBase<double>{-0.4, 0.6, -1.0};
        rig.directional.push_back(key);
        PointLight p{};
        p.position = Vector3DBase<double>{1.5 * std::sin(app.angle()),
            3.0, -3.0 + 1.5 * std::cos(app.angle())};
        p.range = 30.0;
        rig.point.push_back(p);

        Rasterizer rz{fb};
        const auto viewProj = defaultViewProj(app);
        Object4D cube = app.cube();
        auto cm = SGE::Math::translation(0.0, 1.6, 3.5)
            .mul(SGE::Math::rotationY(app.angle()))
            .mul(SGE::Math::rotationX(0.4));
        auto cnrm = SGE::Math::normalMatrix(cm);
        ShadingContext ctx{&rig, app.camera().position};
        auto ct = Pipeline::projectObject(cube, cm, viewProj, cnrm, 800, 600);
        SGE::Render::TileRenderer tiled{fb};
        tiled.drawTextured(ct, diffuse, &ctx);

        drawLightMarker(app, rz, p.position, Color32{255, 250, 220, 255});
    }
    const char *name() const override { return "Light Maps (diffuse tex)"; }
    const char *group() const override { return "Light"; }
};

}
