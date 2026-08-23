#pragma once

#include "../SceneUtil.hpp"
#include "Render/PostProcess.hpp"

#include <cmath>

namespace SGE::Samples {

class DeferScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        static FrameBuffer albedoFb{800, 600};
        static FrameBuffer normalFb{800, 600};
        const double ang = app.angle();
        auto drawGbuffers = [&](){
            albedoFb.clear(0xFF000000u);
            normalFb.clear(0xFF8080FFu);
            SGE::Render::Camera mc{};
            mc.position = Vector3DBase<double>{0, 1.8, -3.6};
            mc.pitch = -0.25;
            auto vp = SGE::Math::perspective(M_PI / 3, 800.0 / 600.0, 0.1, 50.0)
                .mul(mc.viewMatrix());
            struct Obj{ Object4D obj; double x, y, z; };
            std::vector<Obj> objs;
            for(int i = 0; i < 3; i++){
                objs.push_back(Obj{app.cube(), -2.2 + i * 2.2,
                    0.9, 1.0});
            }
            for(auto &o : objs){
                auto cm = SGE::Math::translation(o.x, o.y, o.z)
                    .mul(SGE::Math::rotationY(ang + o.x))
                    .mul(SGE::Math::rotationX(0.4));
                auto cnrm = SGE::Math::normalMatrix(cm);
                auto tris = Pipeline::projectObject(o.obj, cm, vp, cnrm, 800, 600);
                Rasterizer arz{albedoFb};
                for(auto &t : tris){
                    ScreenVertex a = t.v[0], b2 = t.v[1], c2 = t.v[2];
                    Color32 tint{90 + (o.x > 0 ? 120 : 30), 140, 220, 255};
                    a.color = tint; b2.color = tint; c2.color = tint;
                    arz.drawTriangleSolid(a, b2, c2);
                }
                Rasterizer nrz{normalFb};
                for(auto &t : tris){
                    ScreenVertex a = t.v[0], b2 = t.v[1], c2 = t.v[2];
                    auto enc = [](const ScreenVertex &sv){
                        return Color32{
                            static_cast<int32_t>((sv.nx * 0.5 + 0.5) * 255),
                            static_cast<int32_t>((sv.ny * 0.5 + 0.5) * 255),
                            static_cast<int32_t>((sv.nz * 0.5 + 0.5) * 255), 255};
                    };
                    a.color = enc(a); b2.color = enc(b2); c2.color = enc(c2);
                    nrz.drawTriangleSolid(a, b2, c2);
                }
            }
        };
        drawGbuffers();
        // lighting pass from G-buffer
        const Vector3DBase<double> L{-0.5, 0.75, -1.0};
        SGE::Render::RunPass(normalFb, fb,
            [&](double u, double v, const FrameBuffer &norms){
            const auto n = SGE::Render::fetchRGB(norms, u, v);
            const auto alb = SGE::Render::fetchRGB(albedoFb, u, v);
            if(alb.r + alb.g + alb.b < 1e-6){
                return Color32{18, 18, 26, 255};
            }
            Vector3DBase<double> N{n.r / 255.0 * 2.0 - 1.0,
                n.g / 255.0 * 2.0 - 1.0, n.b / 255.0 * 2.0 - 1.0};
            N = N.normalize();
            const double diff = std::max(0.0, N.dot(L.normalize()));
            const float outR = static_cast<float>(alb.r * (0.15 + 0.85 * diff));
            const float outG = static_cast<float>(alb.g * (0.15 + 0.85 * diff));
            const float outB = static_cast<float>(alb.b * (0.15 + 0.85 * diff));
            return Color32{static_cast<int32_t>(std::min(255.0f, outR)),
                           static_cast<int32_t>(std::min(255.0f, outG)),
                           static_cast<int32_t>(std::min(255.0f, outB)), 255};
        });
    }
    const char *name() const override { return "Deferred Shading (G-buffer)"; }
    const char *group() const override { return "LightAdv"; }
};

}
