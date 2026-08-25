#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>
#include <vector>

namespace SGE::Samples {

class MultiInstanceScene final : public IScene {
public:
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        Rasterizer rz{fb};
        static Object4D proto = SGE::Render::MakeSphere(1.0, 12, 8);
        // reference: offsets {-18,-14..14}^2 gap4, size grows with id,
        // red channel rises with id; model T(0,0,-3) scale 0.3
        int id = 0;
        for(int ox = 0; ox < 10; ox++){
            for(int oy = 0; oy < 10; oy++){
                if(id >= 100){ break; }
                Object4D ball = proto;
                const double sc = 0.35 + 0.15 * (id % 10) / 9.0;
                auto bm = SGE::Math::translation(
                    ox * 1.3 - 5.85, oy * 1.3 - 5.85,
                    static_cast<double>(-8))
                    .mul(SGE::Math::scale(sc, sc, sc));
                auto bnrm = SGE::Math::normalMatrix(bm);
                auto bt = Pipeline::projectObject(ball, bm,
                    refViewProj(app.camera()), bnrm, g_renderW, g_renderH);
                const int rc = std::min(255, id * 5);
                for(auto &t : bt){
                    ScreenVertex r0 = t.v[0], r1 = t.v[1], r2 = t.v[2];
                    r0.color = Color32{rc, 0, 0, 255};
                    r1.color = Color32{rc, 0, 0, 255};
                    r2.color = Color32{rc, 0, 0, 255};
                    rz.drawTriangleWireframe(r0, r1, r2);
                }
                id++;
            }
        }
    }
    void drawUi(Application &) override {
        ImGui::Text("100 wireframe spheres, growing & reddening");
    }
    const char *name() const override { return "Instanced Sphere Grid"; }
    const char *group() const override { return "Advanced"; }
};

}
