#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>

namespace SGE::Samples {

class CameraWalkScene final : public IScene {
public:
    int m_count{10};
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        Rasterizer rz{fb};
        static Texture dog = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/dog.jpg");
        static const double cubePos[10][3] = {
            {0.0, 0.0, 0.0},      {2.0, 5.0, -15.0},
            {-1.5, -2.2, -2.5},   {-3.8, -2.0, -12.3},
            {2.4, -0.4, -3.5},    {-1.7, 3.0, -7.5},
            {1.3, -2.0, -2.5},    {1.5, 2.0, -2.5},
            {1.5, 0.2, -1.5},     {-1.3, 1.0, -1.5}};
        const auto vp = refViewProj(app.camera());
        for(int i = 0; i < m_count; i++){
            Object4D cube = unitCube(app);
            const double angDeg = 20.0 * (i + 1) * app.angle();
            const double a = angDeg * M_PI / 180.0;
            auto rotAxis = SGE::Math::rotationY(a)
                .mul(SGE::Math::rotationX(a * 0.3))
                .mul(SGE::Math::rotationZ(a * 0.5));
            auto m = SGE::Math::translation(cubePos[i][0], cubePos[i][1],
                                            cubePos[i][2]).mul(rotAxis);
            auto nrm = SGE::Math::normalMatrix(m);
            auto tris = Pipeline::projectObject(cube, m, vp, nrm, g_renderW, g_renderH);
            for(auto &t : tris){
                rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                        dog, nullptr, TextureFilter::Bilinear,
                                        TextureWrap::Clamp);
            }
        }
    }
    void drawUi(Application &) override {
        ImGui::Begin("Settings");
        ImGui::SetNextItemWidth(200);
        ImGui::SliderInt("Cube Count", &m_count, 1, 10);
        ImGui::End();
    }
    const char *name() const override { return "Camera Walkthrough"; }
    const char *group() const override { return "Base"; }
};

}
