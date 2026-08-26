#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>

namespace SGE::Samples {

class CullFaceScene final : public IScene {
public:
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
    }
    bool m_cull{true};
    int m_grassCount{10};
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        static Texture marble = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/marble.jpg");
        Rasterizer rz{fb};
        rz.cullBackface = m_cull;
        // reference: T(0,0,-4) RotX45 RotY45 scale 2
        Object4D cube = app.cube();
        auto cm = SGE::Math::translation(0.0, 0.0, -4.0)
            .mul(SGE::Math::rotationX(M_PI / 4))
            .mul(SGE::Math::rotationY(M_PI / 4));
        auto cnrm = SGE::Math::normalMatrix(cm);
        auto ct = Pipeline::projectObject(cube, cm,
            refViewProj(app.camera()), cnrm, g_renderW, g_renderH);
        for(auto &t : ct){
            rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                    marble, nullptr, TextureFilter::Bilinear,
                                    TextureWrap::Clamp);
        }
    }
    void drawUi(Application &) override {
        ImGui::Begin("Settings");
        ImGui::SetNextItemWidth(200);
        ImGui::SliderInt("Grass Count", &m_grassCount, 1, 10);
        ImGui::End();
    }
    const char *name() const override { return "Backface Culling"; }
    const char *group() const override { return "Advanced"; }
};

}
