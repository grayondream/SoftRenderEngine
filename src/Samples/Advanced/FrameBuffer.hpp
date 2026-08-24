#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"
#include "Render/PostProcess.hpp"

#include <cmath>
#include <vector>

namespace SGE::Samples {

class FrameBufferScene final : public IScene {
public:
    void setup(Application &app) override {
        resetCamera(app, -1.0, -1.5, 3.5, 0.0, -10.0 * M_PI / 180.0);
    }
    int m_effect{0};   // 0 none, 1 inversion, 2 gray, 3 sharpen
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF1A1A1Au);
        static Texture container = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/container2.jpg");
        static Texture metal = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/metal.png");
        LightingRig rig{};
        rig.ambient = 0.85f;
        ShadingContext ctx{&rig, app.camera().position};
        auto cam = refCamera(-1.0, -1.5, 3.5);
        cam.pitch = -10.0 * M_PI / 180.0;
        const auto vp = refViewProj(cam);
        SGE::Render::TileRenderer tiled{fb};
        Object4D cube = unitCube(app);
        // reference: 64-cube grid (x*1.1, y*1.1-2, z*1.1-5), x,y,z in {-2..1}
        for(int gx = -2; gx <= 1; gx++){
            for(int gy = -2; gy <= 1; gy++){
                for(int gz = -2; gz <= 1; gz++){
                    auto cm = SGE::Math::translation(
                        gx * 1.1, gy * 1.1 - 2.0, gz * 1.1 - 5.0);
                    auto cnrm = SGE::Math::normalMatrix(cm);
                    tiled.drawTextured(Pipeline::projectObject(cube, cm,
                        vp, cnrm, g_renderW, g_renderH), container, &ctx);
                }
            }
        }
        if(m_effect > 0){
            applyFilter(fb, m_effect);
        }
    }
    void drawUi(Application &) override {
        const char *fx[] = {"None", "Inversion", "Gray", "Sharpen"};
        ImGui::Combo("Filter", &m_effect, fx, 4);
    }
    const char *name() const override { return "FrameBuffer Post FX"; }
    const char *group() const override { return "Advanced"; }
private:
    static void applyFilter(FrameBuffer &fb, int effect){
        static std::vector<uint32_t> src;
        src.resize(fb.width() * fb.height());
        std::copy(fb.colorData(), fb.colorData() + src.size(), src.begin());
        const std::size_t w = fb.width(), h = fb.height();
        for(std::size_t y = 0; y < h; y++){
            for(std::size_t x = 0; x < w; x++){
                const uint32_t c = src[y * w + x];
                int r = (c >> 16) & 0xFF, g = (c >> 8) & 0xFF,
                    b = c & 0xFF;
                if(effect == 1){          // inversion
                    r = 255 - r; g = 255 - g; b = 255 - b;
                }else if(effect == 2){    // gray
                    const int gr = static_cast<int>(
                        0.2126 * r + 0.7152 * g + 0.0722 * b);
                    r = g = b = gr;
                }else if(effect == 3){    // sharpen kernel
                                        auto at = [&](std::size_t xx, std::size_t yy){
                        return src[yy * w + xx];
                    };
                    const std::size_t xs[3] = {x > 0 ? x - 1 : x, x,
                        x + 1 < w ? x + 1 : x};
                    const std::size_t ys[3] = {y > 0 ? y - 1 : y, y,
                        y + 1 < h ? y + 1 : h - 1};
                    auto ch = [&](int shift){
                        const int k[3][3] = {
                            {-1,-1,-1},{-1,9,-1},{-1,-1,-1}};
                        double sum = 0;
                        for(int ky = 0; ky < 3; ky++){
                            for(int kx = 0; kx < 3; kx++){
                                sum += k[ky][kx] *
                                    ((at(xs[kx], ys[ky]) >> shift) & 0xFF);
                            }
                        }
                        return sum;
                    };
r = static_cast<int>(std::clamp(ch(16), 0.0, 255.0));
                    g = static_cast<int>(std::clamp(ch(8), 0.0, 255.0));
                    b = static_cast<int>(std::clamp(ch(0), 0.0, 255.0));
                }
                fb.setPixel(x, y,
                    0xFF000000u | (static_cast<uint32_t>(r) << 16)
                    | (static_cast<uint32_t>(g) << 8)
                    | static_cast<uint32_t>(b), -2.0f);
            }
        }
    }
};

}
