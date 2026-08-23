#pragma once

#include "../SceneUtil.hpp"
#include "Log.hpp"
#include "Render/ObjLoader.hpp"

#include <cmath>

namespace SGE::Samples {

class LoadModelScene final : public IScene {
public:
    void setup(Application &) override {
        m_metal = Texture(1, 1, std::vector<uint32_t>{0xFFB8C4D0u}.data());
        m_modelLoaded = loadObjFromFile(
            "assets/models/nanosuit/nanosuit.obj", m_model);
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        LightingRig rig{};
        rig.ambient = 0.28f;
        rig.specularStrength = 0.65f;
        rig.shininess = 32.0f;
        DirectionalLight key{};
        key.direction = Vector3DBase<double>{-0.45, 0.75, -1.0};
        key.color = ColorFlt{1, 1, 1};
        rig.directional.push_back(key);
        PointLight rim{};
        rim.position = Vector3DBase<double>{2.5 * std::sin(app.angle() + 1.0),
            2.0, -1.5 + 2.5 * std::cos(app.angle() + 1.0)};
        rim.color = ColorFlt{0.85f, 0.9f, 1.0f};
        rim.range = 25.0;
        rig.point.push_back(rim);

        ShadingContext ctx{&rig, app.camera().position};
        const auto viewProj = defaultViewProj(app);
        auto model2 = SGE::Math::translation(0.0, -1.2, 4.5)
            .mul(SGE::Math::rotationY(app.angle()));
        auto mnrm = SGE::Math::normalMatrix(model2);
        SGE::Render::TileRenderer tiled{fb};
        auto mt = Pipeline::projectObject(m_model, model2, viewProj, mnrm, 800, 600);
        tiled.drawTextured(mt, m_metal, &ctx);
    }
    void drawUi(Application &) override {
        ImGui::Text("nanosuit.obj — %d verts", static_cast<int>(m_model.numVertices));
    }
    const char *name() const override { return "Load Model (nanosuit)"; }
    const char *group() const override { return "Model"; }
private:
    bool m_modelLoaded{false};
    Object4D m_model{};
    Texture m_metal{};
};

}
