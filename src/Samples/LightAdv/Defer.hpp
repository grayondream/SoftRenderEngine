#pragma once

#include "../SceneUtil.hpp"
#include "Render/PostProcess.hpp"

#include <cmath>
#include <vector>

namespace SGE::Samples {

class DeferScene final : public IScene {
public:
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
        // lighting pass: reference rainbow point lights (subsampled 25 of 100)
        SGE::Render::RunPass(normalFb, fb,
            [&](double u, double v, const FrameBuffer &norms){
            const auto n = SGE::Render::fetchRGB(norms, u, v);
            const auto alb = SGE::Render::fetchRGB(albedoFb, u, v);
            if(alb.r + alb.g + alb.b < 1e-6){
                return Color32{10, 10, 14, 255};
            }
            Vector3DBase<double> N{n.r / 255.0 * 2.0 - 1.0,
                n.g / 255.0 * 2.0 - 1.0, n.b / 255.0 * 2.0 - 1.0};
            N = N.normalize();
            double or2 = 0, og = 0, ob = 0;
            for(int gx = 0; gx < 10; gx += 3){
                for(int gz = 0; gz < 10; gz += 3){
                    const double lx = (gx - 4.5) * 1.0;
                    const double lz = (gz - 4.5) * 1.0 - 2.0;
                    const double dx = lx, dz = lz;
                    const double d2 = dx * dx + dz * dz + 9.0;
                    if(d2 > 60.0){ continue; }   // distance cull
                    const double att = 1.0 / (1.0 + 0.7 * std::sqrt(d2)
                        + 1.8 * d2 * 0.06);
                    const float cr = std::min(1.0f,
                        static_cast<float>(std::abs(lx) / 6.0));
                    const float cg = 0.574f;
                    const float cb = std::min(1.0f,
                        static_cast<float>(std::abs(lz) / 6.0));
                    Vector3DBase<double> L{lx, 3.0, lz};
                    L = L.normalize();
                    or2 += alb.r * att * cr * 30.0 * std::max(0.0, N.dot(L));
                    og += alb.g * att * cg * 30.0 * std::max(0.0, N.dot(L));
                    ob += alb.b * att * cb * 30.0 * std::max(0.0, N.dot(L));
                }
            }
            return Color32{
                static_cast<int32_t>(std::min(255.0, alb.r * 0.1 + or2)),
                static_cast<int32_t>(std::min(255.0, alb.g * 0.1 + og)),
                static_cast<int32_t>(std::min(255.0, alb.b * 0.1 + ob)), 255};
        });
    }
    void drawUi(Application &) override {
        ImGui::Text("G-buffer: 100 cubes + rainbow point grid");
    }
    const char *name() const override { return "Deferred Shading (G-buffer)"; }
    const char *group() const override { return "LightAdv"; }
};

}
