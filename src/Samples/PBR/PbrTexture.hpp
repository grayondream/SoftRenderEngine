#pragma once

#include "../SceneUtil.hpp"
#include "Render/PostProcess.hpp"

#include <cmath>

namespace SGE::Samples {

class PbrTextureScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF0A0A10u);
        static Texture albedo = SGE::Render::ImageLoader::loadTexture("assets/textures/rusted_iron/albedo.png");
        static Texture metallic = SGE::Render::ImageLoader::loadTexture("assets/textures/rusted_iron/metallic.png");
        static Texture roughness = SGE::Render::ImageLoader::loadTexture("assets/textures/rusted_iron/roughness.png");
        LightingRig rig{};
        rig.ambient = 0.06f;
        PointLight p1{};
        p1.position = Vector3DBase<double>{2.5 * std::sin(app.angle()), 2.2,
            -1.0 + 2.5 * std::cos(app.angle())};
        p1.range = 25.0;
        rig.point.push_back(p1);
        PointLight p2{};
        p2.position = Vector3DBase<double>{-2.0, 3.0, -4.0};
        p2.color = ColorFlt{0.6f, 0.75f, 1.0f};
        p2.range = 25.0;
        rig.point.push_back(p2);

        Rasterizer rz{fb};
        PbrMaterial mat{};
        mat.albedoTex = &albedo;
        mat.metallicTex = &metallic;
        mat.roughnessTex = &roughness;
        ShadingContext ctx{&rig, app.camera().position,
            nullptr, nullptr, nullptr, nullptr, &mat};
        const auto viewProj = defaultViewProj(app);
        Object4D sphere = SGE::Render::MakeSphere(1.1, 26, 18);
        auto sm = SGE::Math::translation(0.0, 1.3, 3.0)
            .mul(SGE::Math::rotationY(app.angle() * 0.7));
        auto snrm = SGE::Math::normalMatrix(sm);
        auto st = Pipeline::projectObject(sphere, sm, viewProj, snrm, 800, 600);
        SGE::Render::TileRenderer tiled{fb};
        tiled.drawTextured(st, app.checker(), &ctx);
        drawLightMarker(app, rz, p1.position, Color32{255, 240, 210, 255});
    }
    const char *name() const override { return "PBR Textured Sphere (rusted iron)"; }
    const char *group() const override { return "PBR"; }
};

}
