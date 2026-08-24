#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>

namespace SGE::Samples {

class MSAAScene final : public IScene {
public:
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
    }
public:
    bool m_gray{false};
    bool m_4x{false};
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF1A1A1Au);
        static Texture dog = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/dog.jpg");
        Rasterizer rz{fb};
        Object4D cube = app.cube();
        auto cm = SGE::Math::translation(0.0, 0.0, -4.0 + 4.0)
            .mul(SGE::Math::rotationX(M_PI / 4))
            .mul(SGE::Math::rotationY(M_PI / 4));
        auto cnrm = SGE::Math::normalMatrix(cm);
        auto ct = Pipeline::projectObject(cube, cm,
            refViewProj(app.camera()), cnrm, 800, 600);
        for(auto &t : ct){
            rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                    dog, nullptr, TextureFilter::Bilinear,
                                    TextureWrap::Clamp);
        }
        if(m_gray){
            for(std::size_t y = 0; y < 600; y++){
                for(std::size_t x = 0; x < 800; x++){
                    const uint32_t c =
                        fb.colorData()[y * 800 + x];
                    const int g = static_cast<int>(
                        0.2126 * ((c >> 16) & 0xFF)
                        + 0.7152 * ((c >> 8) & 0xFF)
                        + 0.0722 * (c & 0xFF));
                    fb.setPixel(x, y, 0xFF000000u
                        | (static_cast<uint32_t>(g) << 16)
                        | (static_cast<uint32_t>(g) << 8)
                        | static_cast<uint32_t>(g), -2.0f);
                }
            }
        }
    }
    void drawUi(Application &) override {
        ImGui::Checkbox("Grayscale", &m_gray);
        ImGui::Checkbox("4x Supersampling", &m_4x);
        ImGui::Text("watch diagonal edges (2x SSAA)");
    }
    const char *name() const override { return "Anti-aliasing (2x SSAA)"; }
    const char *group() const override { return "Advanced"; }
};

}
