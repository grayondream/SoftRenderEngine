#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>
#include <vector>

namespace SGE::Samples {

class ExplodeScene final : public IScene {
public:
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        Rasterizer rz{fb};
        // gradient vertex-color sphere (reference: color from local coords)
        static Object4D sphere = SGE::Render::MakeSphere(1.0, 30, 18);
        // bake vertex colors into polys
        for(int pi2 = 0; pi2 < static_cast<int>(sphere.numPolys); pi2++){
            auto &poly = sphere.plist[static_cast<std::size_t>(pi2)];
            for(int k = 0; k < 3; k++){
                const auto &v = poly.vlist[k];
                poly.color = Color32{
                    static_cast<int32_t>(((v.x / 2.0) / 2.0 + 0.5) * 255.0),
                    static_cast<int32_t>(((v.y / 2.0) / 2.0 + 0.5) * 255.0),
                    static_cast<int32_t>(((v.z / 2.0) / 2.0 + 0.5) * 255.0),
                    255};
            }
        }
        const double mag = 2.0 * ((std::sin(app.angle()) + 1.0) / 2.0);
        Object4D boom = sphere;
        // push each triangle out along its own normal
        std::snprintf(boom.name, sizeof(boom.name), "%s", "boom");
        for(int pi2 = 0; pi2 < static_cast<int>(boom.numPolys); pi2++){
            auto &poly = boom.plist[static_cast<std::size_t>(pi2)];
            Vector3DBase<double> e1{poly.vlist[1].x - poly.vlist[0].x,
                poly.vlist[1].y - poly.vlist[0].y,
                poly.vlist[1].z - poly.vlist[0].z};
            Vector3DBase<double> e2{poly.vlist[2].x - poly.vlist[0].x,
                poly.vlist[2].y - poly.vlist[0].y,
                poly.vlist[2].z - poly.vlist[0].z};
            Vector3DBase<double> n{e1.y * e2.z - e2.y * e1.z,
                e2.x * e1.z - e1.x * e2.z,
                e1.x * e2.y - e2.x * e1.y};
            if(n.length() > 1e-12){
                n = n.normalize();
                n = Vector3DBase<double>{n.x * mag, n.y * mag, n.z * mag};
                for(int k = 0; k < 3; k++){
                    poly.vlist[k].x += n.x;
                    poly.vlist[k].y += n.y;
                    poly.vlist[k].z += n.z;
                }
            }
        }
        auto sm = SGE::Math::translation(0.0, 0.0, -3.0)
            .mul(SGE::Math::translation(0.0, 0.0, 3.0))
            .mul(SGE::Math::translation(0.0, 0.8, 0.0))
            .mul(SGE::Math::scale(2.0, 2.0, 2.0));
        auto snrm = SGE::Math::normalMatrix(sm);
        auto st = Pipeline::projectObject(boom, sm,
            refViewProj(app.camera()), snrm, g_renderW, g_renderH);
        for(auto &t : st){
            rz.drawTriangleSolid(t.v[0], t.v[1], t.v[2]);
        }
    }
    void drawUi(Application &) override {
        ImGui::Begin("Settings");
        ImGui::End();
    }
    const char *name() const override { return "Explode (face normals)"; }
    const char *group() const override { return "Advanced"; }
};

}
