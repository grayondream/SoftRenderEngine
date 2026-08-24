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
        for(int ox = -18; ox <= 14; ox += 4){
            for(int oy = -18; oy <= 14; oy += 4){
                if(id >= 100){ break; }
                Object4D ball = proto;
                auto bm = SGE::Math::translation(
                    static_cast<double>(ox) * 0.3 + 1.0,
                    static_cast<double>(oy) * 0.3 - 2.5, -3.0)
                    .mul(SGE::Math::scale(0.3 * (id / 100.0 + 0.15),
                                          0.3 * (id / 100.0 + 0.15),
                                          0.3 * (id / 100.0 + 0.15)));
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
