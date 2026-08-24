#pragma once

#include "../SceneUtil.hpp"
#include "Render/PostProcess.hpp"

#include <cmath>
#include <vector>

namespace SGE::Samples {

class DeferScene final : public IScene {
public:
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.5, 3.0);
    }

    void render(Application &app) override {
        auto &fb = app.framebuffer();
        static FrameBuffer albedoFb{800, 600};
        static FrameBuffer normalFb{800, 600};
        auto cam = refCamera(0, 0.5 + 4.0, 3.0);
        cam.pitch = -0.45;
        const auto vp = refViewProj(cam);
        // G-buffer pass: 100 small wood cubes 10x10 gap1 S0.1 over brick plane
        auto drawGbuffers = [&](){
            albedoFb.clear(kRefClear);
            normalFb.clear(0xFF8080FFu);
            Object4D ground = refPlane(Color32{255,255,255,255});
            std::snprintf(ground.name, sizeof(ground.name), "%s", "ground");
            {
                Rasterizer arz{albedoFb};
                Rasterizer nrz{normalFb};
                auto gm = SGE::Math::translation(0.0, -0.5, -2.0);
                auto gnrm = SGE::Math::normalMatrix(gm);
                auto gt = Pipeline::projectObject(ground, gm,
                    vp, gnrm, 800, 600);
                for(auto &t : gt){
                    ScreenVertex a = t.v[0], b2 = t.v[1], c2 = t.v[2];
                    a.color = Color32{150, 90, 60, 255};
                    b2.color = a.color; c2.color = a.color;
                    arz.drawTriangleSolid(a, b2, c2);
                    auto enc = [](const ScreenVertex &sv){
                        return Color32{
                            static_cast<int32_t>((sv.nx * 0.5 + 0.5) * 255),
                            static_cast<int32_t>((sv.ny * 0.5 + 0.5) * 255),
                            static_cast<int32_t>((sv.nz * 0.5 + 0.5) * 255),
                            255};
                    };
                    a.color = enc(a); b2.color = enc(b2); c2.color = enc(c2);
                    nrz.drawTriangleSolid(a, b2, c2);
                }
            }
        };
        drawGbuffers();
        // lighting pass: reference rainbow point lights (subsampled 16 of 100)
        const std::size_t gw = normalFb.width();
        const auto *npx = normalFb.colorData();
        const auto *apx = albedoFb.colorData();
        for(std::size_t y2 = 0; y2 < 600; y2++){
            for(std::size_t x2 = 0; x2 < 800; x2++){
            const uint32_t nc = npx[y2 * gw + x2];
            const uint32_t ac = apx[y2 * gw + x2];
            Vector3DBase<double> N{
                ((nc >> 16) & 0xFF) / 255.0 * 2.0 - 1.0,
                ((nc >> 8) & 0xFF) / 255.0 * 2.0 - 1.0,
                (nc & 0xFF) / 255.0 * 2.0 - 1.0};
            N = N.normalize();
            double or2 = static_cast<double>((ac >> 16) & 0xFF) * 0.1;
            double og = static_cast<double>((ac >> 8) & 0xFF) * 0.1;
            double ob = static_cast<double>(ac & 0xFF) * 0.1;
            if(or2 + og + ob > 0.5){
                for(int gx = 0; gx < 10; gx += 3){
                    for(int gz = 0; gz < 10; gz += 3){
                        const double lx = (gx - 4.5) * 1.0;
                        const double lz = (gz - 4.5) * 1.0 - 2.0;
                        const double dx = lx, dz = lz;
                        const double d2 = dx * dx + dz * dz + 9.0;
                        if(d2 > 60.0){ continue; }
                        const double att = 1.0 /
                            (1.0 + 0.7 * std::sqrt(d2) + 1.8 * d2 * 0.06);
                        const float cr = std::min(1.0f,
                            static_cast<float>(std::abs(lx) / 6.0));
                        const float cg = 0.574f;
                        const float cb = std::min(1.0f,
                            static_cast<float>(std::abs(lz) / 6.0));
                        Vector3DBase<double> L{lx, 3.0, lz};
                        L = L.normalize();
                        const double ndl = std::max(0.0, N.dot(L));
                        or2 += static_cast<double>(
                            (ac >> 16) & 0xFF) / 255.0 * att * cr
                            * 30.0 * ndl * 255.0 * 0.85;
                        og += static_cast<double>(
                            (ac >> 8) & 0xFF) / 255.0 * att * cg
                            * 30.0 * ndl * 255.0 * 0.85;
                        ob += static_cast<double>(ac & 0xFF) / 255.0
                            * att * cb * 30.0 * ndl * 255.0 * 0.85;
                    }
                }
            }
            fb.setPixel(x2, y2,
                0xFF000000u
                | (static_cast<uint32_t>(std::min(255.0, or2)) << 16)
                | (static_cast<uint32_t>(std::min(255.0, og)) << 8)
                | static_cast<uint32_t>(std::min(255.0, ob)), -2.0f);
            }
        }
    }
    void drawUi(Application &) override {
        ImGui::Text("G-buffer: 100 cubes + rainbow point grid");
    }
    const char *name() const override { return "Deferred Shading (G-buffer)"; }
    const char *group() const override { return "LightAdv"; }
};

}
