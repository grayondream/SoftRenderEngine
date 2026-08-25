#pragma once

#include "../SceneUtil.hpp"
#include "Render/ObjLoader.hpp"
#include "Render/AtlasBuilder.hpp"

#include <cmath>
#include <vector>

namespace SGE::Samples {

// GL AppType: LoadModel — nanosuit.obj with real MTL diffuse atlas
class LoadModelScene final : public IScene {
public:
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.5, 6.0, 3.14159265358979, -0.12);
        m_loaded = loadObjMultiMaterial(
            "assets/models/nanosuit/nanosuit.obj",
            m_chunks, m_faceMtl, m_mats);
        if(m_loaded){
            m_atlas = SGE::Render::BuildDiffuseAtlas(m_mats);
            // remap each polygon's UV into its material's atlas tile
            int face = 0;
            for(auto &c : m_chunks){
                for(int pi2 = 0; pi2 < static_cast<int>(c.numPolys);
                    pi2++, face++){
                    const int mi = face < static_cast<int>(m_faceMtl.size())
                        ? m_faceMtl[static_cast<std::size_t>(face)] : 0;
                    if(mi >= static_cast<int>(m_atlas.tiles.size())){
                        continue;
                    }
                    const auto &t = m_atlas.tiles[
                        static_cast<std::size_t>(mi)];
                    auto &poly =
                        c.plist[static_cast<std::size_t>(pi2)];
                    for(int k = 0; k < 3; k++){
                        poly.uvlist[k].u = t.u0 +
                            poly.uvlist[k].u * (t.u1 - t.u0);
                        poly.uvlist[k].v = t.v0 +
                            poly.uvlist[k].v * (t.v1 - t.v0);
                    }
                }
            }
        }
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        if(!m_loaded){ return; }
        // reference: unlit direct-out, camera z=6
        ShadingContext ctx{};
        auto cam = refCamera(0, 0.5, 6.0);
        cam.pitch = -0.12;
        const auto vp = refViewProj(cam);
        auto model2 = SGE::Math::translation(0.0, -1.4, 4.5)
            .mul(SGE::Math::rotationY(app.angle() * 0.3));
        auto mnrm = SGE::Math::normalMatrix(model2);
        SGE::Render::TileRenderer tiled{fb};
        for(const auto &c : m_chunks){
            auto mt = Pipeline::projectObject(c, model2,
                vp, mnrm, g_renderW, g_renderH);
            tiled.drawTextured(mt, m_atlas.texture, &ctx);
        }
    }
    void drawUi(Application &) override {
        ImGui::Text("nanosuit.obj — %d chunks, %d materials",
                    static_cast<int>(m_chunks.size()),
                    static_cast<int>(m_mats.names.size()));
    }
    const char *name() const override { return "Load Model (nanosuit MTL)"; }
    const char *group() const override { return "Model"; }
private:
    bool m_loaded{false};
    std::vector<Object4D> m_chunks{};
    std::vector<int> m_faceMtl{};
    ObjMaterialInfo m_mats{};
    SGE::Render::DiffuseAtlas m_atlas{};
};

}
