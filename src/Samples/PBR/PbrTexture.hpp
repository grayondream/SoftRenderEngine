#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <algorithm>
#include <cmath>

namespace SGE::Samples {

class PbrTextureScene final : public IScene {
public:
public:
    void setup(Application &app) override {
        m_white = Texture(1, 1, std::vector<uint32_t>{0xFFFFFFFFu}.data());
        resetCamera(app, 0.0, 0.0, 4.5);
    }
private:
    Texture m_white{};
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        static Texture albedoT = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/rusted_iron/albedo.png");
        static Texture metallicT = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/rusted_iron/metallic.png");
        static Texture roughnessT = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/rusted_iron/roughness.png");

        LightingRig rig{};
        rig.ambient = 0.03f;
        // reference: 4 point lights (±10,±10,10)*300 with 1/d^2 falloff
        const double lp[4][3] = {{-10,10,10},{10,10,10},{-10,-10,10},{10,-10,10}};
        for(int i = 0; i < 4; i++){
            PointLight p{};
            p.position = Vector3DBase<double>{lp[i][0], lp[i][1], lp[i][2]};
            p.range = 200.0;
            rig.point.push_back(p);
        }
        Rasterizer rz{fb};
        const auto vp = refViewProj(app.camera());
        int idx = 0;
        for(int row = -2; row <= 2; row++){
            for(int col = -2; col <= 2; col++){
                PbrMaterial mat{};
                mat.albedoTex = &albedoT;
                mat.metallicTex = &metallicT;
                mat.roughnessTex = &roughnessT;
                mat.metallic = app.pbrMetallic();
                mat.roughness = std::max(0.05f, app.pbrRoughness());
                ShadingContext ctx{&rig, app.camera().position,
                    nullptr, nullptr, nullptr, nullptr, &mat};
                static Object4D protoBall = SGE::Render::MakeSphere(1.0, 24, 16);
                Object4D ball = protoBall;
                const double sc = 0.4;
                for(int vi = 0; vi < static_cast<int>(ball.numVertices); vi++){
                    ball.vlistLocal[static_cast<std::size_t>(vi)].x =
                        protoBall.vlistLocal[static_cast<std::size_t>(vi)].x * sc;
                    ball.vlistLocal[static_cast<std::size_t>(vi)].y =
                        protoBall.vlistLocal[static_cast<std::size_t>(vi)].y * sc;
                    ball.vlistLocal[static_cast<std::size_t>(vi)].z =
                        protoBall.vlistLocal[static_cast<std::size_t>(vi)].z * sc;
                }
                auto bm = SGE::Math::translation(
                    static_cast<double>(col) * 1.0,
                    static_cast<double>(row) * 1.05, 0.0);
                auto bnrm = SGE::Math::normalMatrix(bm);
                auto bt = Pipeline::projectObject(ball, bm, vp, bnrm, g_renderW, g_renderH);
                for(auto &t : bt){
                    rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                            m_white, &ctx);
                }
                (void)idx;
                idx++;
            }
        }
    }
    void drawUi(Application &) override {
        ImGui::Text("rusted_iron albedo/metallic/roughness maps");
    }
    const char *name() const override { return "PBR Textured Sphere (rusted iron)"; }
    const char *group() const override { return "PBR"; }
};

}
