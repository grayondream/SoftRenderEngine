#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

namespace SGE::Samples {

class SimpleTextureScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        Rasterizer rz{fb};
        static Texture dog = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/dog.jpg");
        // reference: NDC quad drawn directly (no projection), centered,
        // spanning most of the viewport, image upright
        const double hw = g_renderW * 0.42;
        const double hh = g_renderH * 0.42;
        const double cx = g_renderW * 0.5;
        const double cy = g_renderH * 0.5;
        ScreenVertex TR{cx + hw, cy - hh, 0.5f, 1};
        ScreenVertex BR{cx + hw, cy + hh, 0.5f, 1};
        ScreenVertex BL{cx - hw, cy + hh, 0.5f, 1};
        ScreenVertex TL{cx - hw, cy - hh, 0.5f, 1};
        // UV: top-left of image at TL (flip V because loader stores top-down)
        TR.u = 1; TR.v = 1;
        BR.u = 1; BR.v = 0;
        BL.u = 0; BL.v = 0;
        TL.u = 0; TL.v = 1;
        const Color32 white{255, 255, 255, 255};
        TR.color = white; BR.color = white;
        BL.color = white; TL.color = white;
        rz.drawTriangleTextured(TL, BL, BR, dog, nullptr,
                                TextureFilter::Bilinear, TextureWrap::Clamp);
        rz.drawTriangleTextured(BR, TL, BL, dog, nullptr,
                                TextureFilter::Bilinear, TextureWrap::Clamp);
    }
    void drawUi(Application &) override {
        ImGui::Text("dog.jpg centered, unlit");
    }
    const char *name() const override { return "Simple Texture"; }
    const char *group() const override { return "Base"; }
};

}
