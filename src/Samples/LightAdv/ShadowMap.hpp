#pragma once

#include "../SceneUtil.hpp"
#include "Render/Shadow.hpp"

#include <cmath>

namespace SGE::Samples {

class ShadowMapScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        auto &cam = app.camera();
        auto rig = makeDefaultRig();
        auto fog = defaultFog(app);
        const double ang = app.angle();

        fb.clear(0xFF000000u);
        Rasterizer rz{fb};
        const auto viewProj = defaultViewProj(app);
        const auto nrm = SGE::Math::normalMatrix(
            SGE::Math::translation(0.0, 0.0, 0.0));

        FrameBuffer shadowMap{256, 256};
        shadowMap.clear();
        Object4D ground = makePlane(6.0, -2.0,
            Vector3DBase<double>{0, 1, 0}, Color32{210, 210, 220, 255});
        std::snprintf(ground.name, sizeof(ground.name), "%s", "ground");

        Object4D obstacleCone = SGE::Render::MakeCone(0.8, 2.0);
        obstacleCone.worldPos = Point4D{-3.0, -0.5, 1.5, 1};
        Object4D obstacleSphere = SGE::Render::MakeSphere(1.0, 24, 16);
        obstacleSphere.worldPos = Point4D{3.0, -0.3, -1.0, 1};

        const Vector3DBase<double> lightPos{6.5, 6.0, 5.0};
        const auto lightVP = SGE::Render::pointLightVP(lightPos,
            Vector3DBase<double>{0, -1.5, 0}, M_PI / 2, 1.0, 0.5, 60.0);

        SpotLight spot{};
        spot.position = lightPos;
        spot.direction = Vector3DBase<double>{-6.5, -7.5, -5.0};
        spot.color = ColorFlt{1.0f, 0.97f, 0.9f};
        spot.range = 40.0;
        spot.cutoffCos = 0.55;
        rig.spot.push_back(spot);

        Object4D obstacleCube = app.cube();
        struct Obstacle{ const Object4D *obj; double x, y, z; double ry; };
        const Obstacle obstacles[] = {
            {&obstacleCube, 0.0, 0.0, 0.0, ang},
            {&obstacleCone, obstacleCone.worldPos.x, obstacleCone.worldPos.y,
             obstacleCone.worldPos.z, ang * 0.5},
            {&obstacleSphere, obstacleSphere.worldPos.x,
             obstacleSphere.worldPos.y + 1.3, obstacleSphere.worldPos.z, 0.0}};
        const auto sceneRot = SGE::Math::rotationY(ang);

        {
            Rasterizer srz{shadowMap};
            auto gtris = Pipeline::projectObject(ground,
                SGE::Math::translation(0.0,0.0,0.0), lightVP, nrm, 256, 256);
            for(auto &t : gtris) srz.drawTriangleDepth(t.v[0], t.v[1], t.v[2]);
            for(const auto &ob : obstacles){
                auto om = SGE::Math::translation(ob.x, ob.y, ob.z)
                    .mul(SGE::Math::rotationY(ob.ry));
                auto onrm = SGE::Math::normalMatrix(om);
                auto ot = Pipeline::projectObject(*ob.obj, om, lightVP, onrm, 256, 256);
                for(auto &t : ot) srz.drawTriangleDepth(t.v[0], t.v[1], t.v[2]);
            }
        }

        SGE::Render::ShadowData sd{&shadowMap, lightVP, 0.004};
        sd.pcfRadius = app.pcfRadius();
        ShadingContext shadCtx{&rig, cam.position,
                               app.fogEnabled() ? &fog : nullptr, &sd};
        {
            SGE::Render::TileRenderer tiled{fb};
            for(const auto &ob : obstacles){
                if(ob.obj == &obstacleCube) continue;
                auto om = SGE::Math::translation(ob.x, ob.y, ob.z)
                    .mul(SGE::Math::rotationY(ob.ry));
                auto onrm = SGE::Math::normalMatrix(om);
                auto ot = Pipeline::projectObject(*ob.obj, om, viewProj, onrm, 800, 600);
                for(auto &t : ot){
                    rz.drawTriangleSolid(t.v[0], t.v[1], t.v[2]);
                }
            }
            auto gtris = Pipeline::projectObject(ground,
                SGE::Math::translation(0.0,0.0,0.0), viewProj, nrm, 800, 600);
            tiled.drawTextured(gtris, app.checker(), &shadCtx);
            auto ctris = Pipeline::projectObject(obstacleCube,
                SGE::Math::translation(0.0, 0.0, 0.0).mul(sceneRot), viewProj,
                SGE::Math::normalMatrix(SGE::Math::rotationY(ang)), 800, 600);
            tiled.drawTextured(ctris, app.checker(), &shadCtx);
        }
    }
    void drawUi(Application &app) override {
        ImGui::SliderInt("PCF Radius", &app.pcfRadius(), 0, 4);
    }
    const char *name() const override { return "Spot Shadow Map + PCF"; }
    const char *group() const override { return "LightAdv"; }
};

}
