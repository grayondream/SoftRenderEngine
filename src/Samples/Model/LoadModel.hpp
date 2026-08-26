#pragma once

#include "../SceneUtil.hpp"
#include "Render/ObjLoader.hpp"
#include "Render/AtlasBuilder.hpp"

#include <vector>

namespace SGE::Samples {

// GL AppType: LoadModel — backpack.obj, unlit diffuse direct-out,
// identity model matrix, default camera at the origin (reference parity)
class LoadModelScene final : public IScene {
public:
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.6, 6.0);
        m_loaded = loadObjMultiMaterial(
            "assets/models/backpack/backpack.obj",
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
        // reference: FragColor = texture(diffuse, uv); model = identity
        ShadingContext ctx{};
        const auto vp = refViewProj(app.camera());
        const auto idm = SGE::Math::translation(0.0, 0.0, 0.0);
        const auto inm = SGE::Math::normalMatrix(idm);
        SGE::Render::TileRenderer tiled{fb};
        for(const auto &c : m_chunks){
            auto mt = Pipeline::projectObject(c, idm,
                vp, inm, g_renderW, g_renderH);
            tiled.drawTextured(mt, m_atlas.texture, &ctx);
        }
    }
    void drawUi(Application &) override {
        ImGui::Begin("Settings");
        ImGui::End();
    }
    const char *name() const override { return "Load Model (backpack)"; }
    const char *group() const override { return "Model"; }
private:
    bool m_loaded{false};
    std::vector<Object4D> m_chunks{};
    std::vector<int> m_faceMtl{};
    ObjMaterialInfo m_mats{};
    SGE::Render::DiffuseAtlas m_atlas{};
};

}
