#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>

namespace SGE::Samples {

class LightMapScene final : public IScene {
public:
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        Rasterizer rz{fb};
        static Texture diffuse = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/container2.jpg");
        static Texture specular = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/container2_specular.jpg");
        const double t = app.angle();
        const Vector3DBase<double> lp{5.0 * std::sin(t), 0.0, 5.0 * std::cos(t)};
        LightingRig rig{};
        rig.ambient = 0.12f;
        rig.specularStrength = 0.6f;
        rig.shininess = 1.0f;
        PointLight p{};
        p.position = lp;
        p.color = ColorFlt{0.6f, 0.6f, 0.6f, 1.0f};
        p.range = 100.0;
        rig.point.push_back(p);
        ShadingContext ctx{&rig, refCamera().position};
        ctx.specTex = &specular;
        Object4D cube = unitCube(app);
        auto cm = SGE::Math::translation(1.0, 0.0, 0.0);
        auto cnrm = SGE::Math::normalMatrix(cm);
        auto ct = Pipeline::projectObject(cube, cm,
            refViewProj(refCamera()), cnrm, g_renderW, g_renderH);
        for(auto &tr : ct){
            rz.drawTriangleTextured(tr.v[0], tr.v[1], tr.v[2],
                                    diffuse, &ctx);
        }
        drawLamp(app, rz, lp);
    }
    void drawUi(Application &) override {
        ImGui::Text("diffuse + specular map cube");
    }
    const char *name() const override { return "Light Map (container2)"; }
    const char *group() const override { return "Light"; }
};

}
