#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>
#include <vector>

namespace SGE::Samples {

class MultiLightsScene final : public IScene {
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
        LightingRig rig{};
        // dir + 4 points + spot (reference LightSourceMult)
        rig.ambient = 0.05f;
        rig.specularStrength = 0.5f;
        rig.shininess = 1.0f;
        DirectionalLight key{};
        key.direction = Vector3DBase<double>{-0.2, -1.0, -0.3};
        key.color = ColorFlt{0.4f, 0.4f, 0.4f, 1.0f};
        rig.directional.push_back(key);
        const double pts[4][3] = {{0,0,-7},{2,2,-10},{-2,-2,-8},{1,0,-9}};
        for(int i = 0; i < 4; i++){
            PointLight p{};
            p.position = Vector3DBase<double>{pts[i][0], pts[i][1], pts[i][2]};
            p.color = ColorFlt{0.8f, 0.8f, 0.8f, 1.0f};
            p.linear = 0.09;
            p.quadratic = 0.032;
            p.range = 30.0;
            rig.point.push_back(p);
        }
        SpotLight spot{};
        spot.position = Vector3DBase<double>{0, 0, -5};
        spot.direction = Vector3DBase<double>{0, 0, -1};
        spot.cutoffCos = std::cos(12.5 * M_PI / 180.0);
        spot.outerCutoffCos = std::cos(15.0 * M_PI / 180.0);
        spot.range = 40.0;
        rig.spot.push_back(spot);
        ShadingContext ctx{&rig, refCamera().position};
        ctx.specTex = &specular;
        SGE::Render::TileRenderer tiled{fb};
        const auto vp = refViewProj(refCamera());
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
        drawLamp(app, rz, Vector3DBase<double>{0, 0, -5});
        drawLamp(app, rz, Vector3DBase<double>{0, 0, -7});
        drawLamp(app, rz, Vector3DBase<double>{2, 2, -10});
        drawLamp(app, rz, Vector3DBase<double>{-2, -2, -8});
        drawLamp(app, rz, Vector3DBase<double>{1, 0, -9});
    }
    void drawUi(Application &) override {
        ImGui::SliderInt("Cube Count", &m_count, 1, 125);
        ImGui::Text("dir + 4 point + spot lights");
    }
    const char *name() const override { return "Multiple Light Sources"; }
    const char *group() const override { return "Light"; }
private:
    int m_count{125};
};

}
