#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>
#include <vector>

namespace SGE::Samples {

class NormalLineScene final : public IScene {
public:
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        Rasterizer rz{fb};
        static Object4D sphere = SGE::Render::MakeSphere(1.0, 20, 12);
        auto sm = SGE::Math::translation(0.0, 0.8, -3.0 + 3.0)
            .mul(SGE::Math::scale(2.0, 2.0, 2.0));
        auto snrm = SGE::Math::normalMatrix(sm);
        const auto vp = refViewProj(app.camera());
        auto st = Pipeline::projectObject(sphere, sm, vp, snrm, g_renderW, g_renderH);
        for(auto &t : st){
            ScreenVertex r0 = t.v[0], r1 = t.v[1], r2 = t.v[2];
            r0.color = Color32{255, 0, 0, 255};
            r1.color = Color32{255, 0, 0, 255};
            r2.color = Color32{255, 0, 0, 255};
            rz.drawTriangleWireframe(r0, r1, r2);
        }
        // yellow normals (world space, magnitude 0.4 scaled by model)
        const double mag = 0.4;
        for(int i = 0; i < static_cast<int>(sphere.numVertices); i += 2){
            const auto &sv = sphere.vlistLocal[static_cast<std::size_t>(i)];
            Vector3DBase<double> n{sv.x, sv.y, sv.z};
            if(n.length() < 1e-9){ continue; }
            n = n.normalize();
            Point4D p0{sv.x * 2.0, sv.y * 2.0, sv.z * 2.0, 1};
            Point4D p1{(sv.x + n.x * mag) * 2.0, (sv.y + n.y * mag) * 2.0,
                       (sv.z + n.z * mag) * 2.0, 1};
            ScreenVertex a{}, b{};
            if(!projectWorldPoint(vp, p0, g_renderW, g_renderH, a)){ continue; }
            if(!projectWorldPoint(vp, p1, g_renderW, g_renderH, b)){ continue; }
            a.color = Color32{255, 255, 0, 255};
            b.color = Color32{255, 255, 0, 255};
            rz.drawLine(a, b);
        }
    }
    void drawUi(Application &) override {
        ImGui::Text("red wireframe + yellow normals");
    }
    const char *name() const override { return "Normal Visualization Lines"; }
    const char *group() const override { return "Advanced"; }
};

}
