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
        fb.clear(kRefClear);
        if(!m_modelLoaded){
            m_modelLoaded = loadObjFromFile(
                "assets/models/nanosuit/nanosuit.obj", m_model);
        }
        // reference: no lights, unlit texture direct out; camera at z=6
        ShadingContext ctx{};
        auto cam = refCamera(0, 0, 6);
        const auto vp = refViewProj(cam);
        auto model2 = SGE::Math::translation(0.0, -1.2, 4.5)
            .mul(SGE::Math::rotationY(app.angle() * 0.3));
        auto mnrm = SGE::Math::normalMatrix(model2);
        SGE::Render::TileRenderer tiled{fb};
        auto mt = Pipeline::projectObject(m_model, model2, vp, mnrm, 800, 600);
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
