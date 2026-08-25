#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>
#include <vector>

namespace SGE::Samples {

class PointLightScene final : public IScene {
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
        Object4D cubeProto = unitCube(app);
        const double t = app.angle();
        const Vector3DBase<double> lp{5.0 * std::sin(t), 0.0, 5.0 * std::cos(t)};
        LightingRig rig{};
        // point light orbiting: amb .2 diff .5 spec 1 shin 1
        rig.ambient = 0.2f;
        rig.specularStrength = 1.0f;
        rig.shininess = 1.0f;
        PointLight p{};
        p.position = lp;
        p.color = ColorFlt{0.5f, 0.5f, 0.5f, 1.0f};
        p.linear = 0.09;
        p.quadratic = 0.032;
        p.range = 40.0;
        rig.point.push_back(p);
        ShadingContext ctx{&rig, app.camera().position};
        ctx.specTex = &specular;
        SGE::Render::TileRenderer tiled{fb};
        const auto vp = refViewProj(app.camera());
        const double rotA = t;  // radians(20 * time)
        int drawn = 0;
        for(int gi = 0; gi < 5 && drawn < m_count; gi++){
            for(int gj = 0; gj < 5 && drawn < m_count; gj++){
                for(int gk = 0; gk < 5 && drawn < m_count; gk++){
                    const double px = (gi - 2) * 2.0;
                    const double py = (gj - 2) * 2.0;
                    const double pz = (gk - 2) * 2.0 - 10.0;
                    auto cm = SGE::Math::translation(px, py, pz)
                        .mul(SGE::Math::rotationY(rotA))
                        .mul(SGE::Math::rotationX(rotA * 0.3))
                        .mul(SGE::Math::rotationZ(rotA * 0.5));
                    auto cnrm = SGE::Math::normalMatrix(cm);
                    auto ct = Pipeline::projectObject(cubeProto, cm,
                        vp, cnrm, g_renderW, g_renderH);
                    tiled.drawTextured(ct, diffuse, &ctx);
                    drawn++;
                }
            }
        }
        drawLamp(app, rz, lp);
    }
    void drawUi(Application &) override {
        ImGui::SliderInt("Cube Count", &m_count, 1, 125);
        ImGui::Text("orbiting point light");
    }
    const char *name() const override { return "Point Light Casters"; }
    const char *group() const override { return "Light"; }
private:
    int m_count{125};
};

}
