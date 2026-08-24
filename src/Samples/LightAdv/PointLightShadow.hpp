#pragma once

#include "../SceneUtil.hpp"
#include "Render/Shadow.hpp"

#include <cmath>

namespace SGE::Samples {

// GL AppType: Shadow_PointLight — omnidirectional cube shadow
class PointLightShadowScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        auto rig = makeDefaultRig();
        auto fog = defaultFog(app);
        const double ang = app.angle();

        fb.clear(0xFF000000u);
        Rasterizer rz{fb};
        const auto viewProj = defaultViewProj(app);
        const auto nrm = SGE::Math::normalMatrix(
            SGE::Math::translation(0.0, 0.0, 0.0));

        Object4D ground = makePlane(7.0, -2.0,
            Vector3DBase<double>{0, 1, 0}, Color32{205, 205, 215, 255}, 14.0);
        std::snprintf(ground.name, sizeof(ground.name), "%s", "ground");

        // point light at room center
        const Vector3DBase<double> lightPos{0.0, 3.2, 2.5};
        PointLight pl{};
        pl.position = lightPos;
        pl.color = ColorFlt{1.0f, 0.96f, 0.88f};
        pl.range = 40.0;
        rig.point.push_back(pl);

        struct Obstacle{ const Object4D *obj; double x, y, z; double ry; };
        Object4D cubeA = app.cube();
        Object4D coneB = SGE::Render::MakeCone(0.7, 1.8);
        Object4D sphereC = SGE::Render::MakeSphere(0.9, 24, 16);
        const Obstacle obstacles[] = {
            {&cubeA, -2.6, -0.8, 2.6, ang},
            {&coneB, 0.0, -1.0, 4.2, ang * 0.7},
            {&sphereC, 2.6, -0.6, 2.2, 0.0}};

        static FrameBuffer faces[6] = {
            FrameBuffer{256, 256}, FrameBuffer{256, 256},
            FrameBuffer{256, 256}, FrameBuffer{256, 256},
            FrameBuffer{256, 256}, FrameBuffer{256, 256}};
        for(int f = 0; f < 6; f++){
            faces[f].clear();
            Rasterizer srz{faces[f]};
            const auto vp = SGE::Render::cubeFaceVP(lightPos, f);
            auto gtris = Pipeline::projectObject(ground,
                SGE::Math::translation(0.0,0.0,0.0), vp, nrm, 256, 256);
            for(auto &t : gtris) srz.drawTriangleDepth(t.v[0], t.v[1], t.v[2]);
            for(const auto &ob : obstacles){
                auto om = SGE::Math::translation(ob.x, ob.y, ob.z)
                    .mul(SGE::Math::rotationY(ob.ry));
                auto onrm = SGE::Math::normalMatrix(om);
                auto ot = Pipeline::projectObject(*ob.obj, om, vp, onrm, 256, 256);
                for(auto &t : ot) srz.drawTriangleDepth(t.v[0], t.v[1], t.v[2]);
            }
        }

        SGE::Render::CubeShadowData cs{};
        cs.lightPos = lightPos;
        cs.farPlane = 30.0;
        cs.bias = 0.006;
        for(int f = 0; f < 6; f++){ cs.faces[f] = &faces[f]; }
        ShadingContext ctx{&rig, app.camera().position,
            app.fogEnabled() ? &fog : nullptr, nullptr, &cs};

        drawLightMarker(app, rz, lightPos, Color32{255, 250, 220, 255});
        SGE::Render::TileRenderer tiled{fb};
        auto gtris = Pipeline::projectObject(ground,
            SGE::Math::translation(0.0,0.0,0.0), viewProj, nrm, 800, 600);
        tiled.drawTextured(gtris, app.checker(), &ctx);
        for(const auto &ob : obstacles){
            auto om = SGE::Math::translation(ob.x, ob.y, ob.z)
                .mul(SGE::Math::rotationY(ob.ry));
            auto onrm = SGE::Math::normalMatrix(om);
            auto ot = Pipeline::projectObject(*ob.obj, om, viewProj, onrm, 800, 600);
            tiled.drawTextured(ot, app.checker(), &ctx);
        }
    }
    void drawUi(Application &) override {
        ImGui::Text("Omnidirectional cube shadow (6-face pass)");
    }
    const char *name() const override { return "Point Light Cube Shadow"; }
    const char *group() const override { return "LightAdv"; }
};

}
