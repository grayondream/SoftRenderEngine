#pragma once

#include "../SceneUtil.hpp"

#include <cstdio>

namespace SGE::Samples {

class ShowcaseEnvScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        auto &cam = app.camera();
        auto rig = makeDefaultRig();
        auto fog = defaultFog(app);

        fb.clear(0xFF000000u);

        SGE::Render::EnvParams mirrorEnv{};
        mirrorEnv.enabled = true;
        mirrorEnv.reflectivity = 0.92;
        SGE::Render::EnvParams glassEnv{};
        glassEnv.enabled = true;
        glassEnv.reflectivity = 0.25;
        glassEnv.refractivity = 0.85;
        glassEnv.ior = 1.52;
        SGE::Render::EnvParams floorEnv{};
        floorEnv.enabled = true;
        floorEnv.reflectivity = 0.18;

        Texture whiteTex(1, 1, std::vector<uint32_t>{0xFFFFFFFFu}.data());
        SGE::Render::TileRenderer tiled{fb};
        const auto viewProj = defaultViewProj(app);
        const auto nrm = SGE::Math::normalMatrix(
            SGE::Math::translation(0.0, 0.0, 0.0));

        Object4D mirrorBall = app.sphere();
        mirrorBall.worldPos = Point4D{-1.6, 0.0, 0, 1};
        auto mm = SGE::Math::translation(mirrorBall.worldPos.x, mirrorBall.worldPos.y + 0.9, mirrorBall.worldPos.z);
        auto mnrm = SGE::Math::normalMatrix(mm);
        ShadingContext mirrorCtx{&rig, cam.position,
            nullptr, nullptr, nullptr, &mirrorEnv};
        auto mt = Pipeline::projectObject(mirrorBall, mm, viewProj, mnrm, 800, 600);
        tiled.drawTextured(mt, whiteTex, &mirrorCtx);

        Object4D crystal = SGE::Render::MakeSphere(0.9, 28, 18);
        crystal.worldPos = Point4D{1.8, -0.1, 0.4, 1};
        auto cm = SGE::Math::translation(crystal.worldPos.x, crystal.worldPos.y + 1.0, crystal.worldPos.z);
        auto cnrm = SGE::Math::normalMatrix(cm);
        ShadingContext crystalCtx{&rig, cam.position,
            nullptr, nullptr, nullptr, &glassEnv};
        auto ct = Pipeline::projectObject(crystal, cm, viewProj, cnrm, 800, 600);
        tiled.drawTextured(ct, whiteTex, &crystalCtx);

        Object4D groundPlane = makePlane(7.0, -2.0,
            Vector3DBase<double>{0, 1, 0}, Color32{190,190,200,255}, 10.0);
        std::snprintf(groundPlane.name, sizeof(groundPlane.name), "%s", "gplane");
        ShadingContext envCtx{&rig, cam.position,
            app.fogEnabled() ? &fog : nullptr, nullptr, nullptr, &floorEnv};
        auto gt = Pipeline::projectObject(groundPlane,
            SGE::Math::translation(0.0, 0.0, 0.0), viewProj, nrm, 800, 600);
        tiled.drawTextured(gt, app.checker(), &envCtx);
    }
    const char *name() const override { return "Env Reflection / Refraction"; }
    const char *group() const override { return "Showcase"; }
};

}
