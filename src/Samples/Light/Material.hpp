#pragma once

#include "../SceneUtil.hpp"

#include <algorithm>
#include <cmath>

namespace SGE::Samples {

class MaterialScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        struct MatDef{
            float amb; float spec; float shine;
            uint32_t tint;
        };
        const MatDef mats[4] = {
            {0.10f, 0.85f, 32.0f, 0xFF8CE633},
            {0.12f, 0.95f, 16.0f, 0xFF598CF2},
            {0.30f, 0.40f, 96.0f, 0xFFE6EDF2},
            {0.08f, 1.00f,  8.0f, 0xFF4026E6}};
        const double pos[4][2] = {{-2.2, 0.6}, {0.6, 0.6}, {-2.2, 3.0}, {0.6, 3.0}};
        const auto viewProj = defaultViewProj(app);
        SGE::Render::TileRenderer tiled{fb};
        for(int i = 0; i < 4; i++){
            LightingRig rig{};
            rig.ambient = mats[i].amb;
            rig.specularStrength = mats[i].spec;
            rig.shininess = mats[i].shine;
            DirectionalLight key{};
            key.direction = Vector3DBase<double>{-0.4, 0.7, -1.0};
            key.color = ColorFlt{1, 1, 1};
            rig.directional.push_back(key);
            PointLight p{};
            p.position = Vector3DBase<double>{2.0, 3.0, -2.5};
            p.range = 25.0;
            rig.point.push_back(p);

            Object4D cube = app.cube();
            auto cm = SGE::Math::translation(pos[i][0], pos[i][1], 2.5)
                .mul(SGE::Math::rotationY(app.angle() * (i + 1) * 0.3));
            auto cnrm = SGE::Math::normalMatrix(cm);
            Texture tint(1, 1, std::vector<uint32_t>{mats[i].tint}.data());
            ShadingContext ctx{&rig, app.camera().position};
            auto ct = Pipeline::projectObject(cube, cm, viewProj, cnrm, 800, 600);
            tiled.drawTextured(ct, tint, &ctx);
        }
    }
    void drawUi(Application &) override {
        ImGui::Text("emerald / copper / pearl / ruby");
    }
    const char *name() const override { return "Material System (4 materials)"; }
    const char *group() const override { return "Light"; }
};

}
