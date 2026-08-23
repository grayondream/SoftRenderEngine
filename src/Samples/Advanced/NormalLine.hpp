#pragma once

#include "../SceneUtil.hpp"
#include "Light/LightUtil.hpp"

#include <cmath>

namespace SGE::Samples {

class NormalLineScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        LightingRig rig{};
        rig.ambient = 0.3f;
        const double a = app.angle();
        Rasterizer rz{fb};
        const auto viewProj = defaultViewProj(app);
        Object4D sphere = app.sphere();
        auto sm = SGE::Math::translation(0.0, 1.4, 3.0).mul(SGE::Math::rotationY(a));
        auto snrm = SGE::Math::normalMatrix(sm);
        ShadingContext ctx{&rig, app.camera().position};
        SGE::Render::TileRenderer tiled{fb};
        auto st = Pipeline::projectObject(sphere, sm, viewProj, snrm, 800, 600);
        tiled.drawTextured(st, app.checker(), &ctx);

        const int stride = 6;
        for(int i = 0; i + 1 < static_cast<int>(sphere.numVertices); i += stride){
            const auto &v = sphere.vlistLocal[static_cast<std::size_t>(i)];
            Vector3DBase<double> n{v.x, v.y, v.z};
            if(n.length() < 1e-9){ continue; }
            n = n.normalize();
            Point4D p0{sm[0][0][0][3] * 0 + v.x, v.y, v.z, 1};
            Point4D q0{0, 0, 0, 1}, q1{0, 0, 0, 1};
            // rotate endpoints by same yaw as sphere model
            const double c = std::cos(a), s = std::sin(a);
            Point4D r0{c * v.x + s * v.z, v.y, -s * v.x + c * v.z, 1};
            Point4D r1{r0.x + n.x * 0.22, r0.y + n.y * 0.22,
                       r0.z + n.z * 0.22, 1};
            (void)p0; (void)q0; (void)q1;
            ScreenVertex s0{}, s1{};
            if(!projectWorldPoint(viewProj,
                    Point4D{r0.x, r0.y, r0.z, 1}, 800, 600, s0)){ continue; }
            if(!projectWorldPoint(viewProj,
                    Point4D{r1.x, r1.y, r1.z, 1}, 800, 600, s1)){ continue; }
            s0.color = Color32{255, 220, 60, 255};
            s1.color = Color32{255, 220, 60, 255};
            rz.drawLine(s0, s1);
        }
    }
    void drawUi(Application &) override {
        ImGui::Text("Yellow segments = vertex normals");
    }
    const char *name() const override { return "Normal Visualization Lines"; }
    const char *group() const override { return "Advanced"; }
};

}
