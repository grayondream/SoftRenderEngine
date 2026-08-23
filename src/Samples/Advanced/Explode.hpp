#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"
#include "Light/LightUtil.hpp"

#include <cmath>

namespace SGE::Samples {

class ExplodeScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        LightingRig rig{};
        rig.ambient = 0.25f;
        rig.specularStrength = 0.4f;
        DirectionalLight key{};
        key.direction = Vector3DBase<double>{-0.4, 0.7, -1.0};
        rig.directional.push_back(key);
        Rasterizer rz{fb};
        const auto viewProj = defaultViewProj(app);
        Object4D cube = app.cube();
        const double t = app.angle() * 2.0;
        const double k = 0.45 * std::sin(t);
        for(int i = 0; i < static_cast<int>(cube.numVertices); i++){
            const auto &v = cube.vlistLocal[i];
            Vector3DBase<double> n{v.x, v.y, v.z};
            if(n.length() > 1e-9){ n = n.normalize(); }
            cube.vlistLocal[i] = Point4D{
                v.x + n.x * k, v.y + n.y * k, v.z + n.z * k, 1};
        }
        auto cm = SGE::Math::translation(0.0, 1.6, 3.0)
            .mul(SGE::Math::rotationY(t * 0.4));
        auto cnrm = SGE::Math::normalMatrix(cm);
        ShadingContext ctx{&rig, app.camera().position};
        SGE::Render::TileRenderer tiled{fb};
        auto ct = Pipeline::projectObject(cube, cm, viewProj, cnrm, 800, 600);
        tiled.drawTextured(ct, app.checker(), &ctx);
    }
    void drawUi(Application &) override {
        ImGui::Text("Vertices pushed along normals (CPU explode)");
    }
    const char *name() const override { return "Explode (CPU vertex offset)"; }
    const char *group() const override { return "Advanced"; }
};

}
