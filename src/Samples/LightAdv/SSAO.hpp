#pragma once

#include "../SceneUtil.hpp"
#include "Render/ObjLoader.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace SGE::Samples {

class SSAOScene final : public IScene {
public:
    bool m_ssaoOn{true};
    void setup(Application &app) override {
        resetCamera(app, 0.0, 1.2, -4.0);
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        LightingRig rig{};
        rig.ambient = 0.3f;
        rig.specularStrength = 0.15f;
        // reference: single bluish point light at (2,4,-2)
        PointLight p{};
        p.position = Vector3DBase<double>{2.0, 4.0, -2.0 + 4.0};
        p.color = ColorFlt{0.2f, 0.2f, 0.7f};
        p.range = 80.0;
        rig.point.push_back(p);

        Rasterizer rz{fb};
        ShadingContext ctx{&rig, app.camera().position};
        const auto vp = refViewProj(app.camera());
        SGE::Render::TileRenderer tiled{fb};

        // room box: inverted cube S15 (interior walls)
        static Object4D room = []{
            Object4D r{};
            std::snprintf(r.name, sizeof(r.name), "%s", "room");
            double s2 = 0.5;
            Point4D v[8] = {{-s2,-s2,-s2,1},{s2,-s2,-s2,1},{s2,s2,-s2,1},
                            {-s2,s2,-s2,1},{-s2,-s2,s2,1},{s2,-s2,s2,1},
                            {s2,s2,s2,1},{-s2,s2,s2,1}};
            for(int i = 0; i < 8; i++){ r.vlistLocal[i] = v[i]; }
            struct F{ int a,b,c; };
            const F fs[12] = {{0,3,2},{0,2,1},{4,5,6},{4,6,7},{0,1,5},{0,5,4},
                              {3,7,6},{3,6,2},{1,2,6},{1,6,5},{0,4,7},{0,7,3}};
            r.numVertices = 8;
            r.numPolys = 12;
            for(int i = 0; i < 12; i++){
                for(int k = 0; k < 3; k++){
                    const int vi = (k == 0 ? fs[i].a :
                                    k == 1 ? fs[i].c : fs[i].b);
                    r.plist[static_cast<std::size_t>(i)].vlist[k] = v[vi];
                }
                r.plist[static_cast<std::size_t>(i)].color =
                    Color32{200, 200, 200, 255};
            }
            return r;
        }();
        auto rm = SGE::Math::translation(0.0, 3.0, 3.0)
            .mul(SGE::Math::scale(9.0, 7.0, 9.0));
        auto rnrm = SGE::Math::normalMatrix(rm);

        if(!m_modelLoaded){
            m_modelLoaded = loadObjFromFile(
                "assets/models/nanosuit/nanosuit.obj", m_model);
            Texture white(1, 1,
                std::vector<uint32_t>{0xFFF2F2F2u}.data());
            m_gray = white;
        }
        auto mm = SGE::Math::translation(0.0, -0.8, 4.0)
            .mul(SGE::Math::rotationY(app.angle() * 0.15))
            .mul(SGE::Math::scale(0.55, 0.55, 0.55));
        auto mnrm = SGE::Math::normalMatrix(mm);

        // draw room first (walls receive AO darkening near contacts)
        tiled.drawTextured(Pipeline::projectObject(room, rm,
            vp, rnrm, g_renderW, g_renderH), m_gray, &ctx);
        tiled.drawTextured(Pipeline::projectObject(m_model, mm,
            vp, mnrm, g_renderW, g_renderH), m_gray, &ctx);

        if(m_ssaoOn){
            // depth-contrast contact shadow approximation
            static std::vector<uint32_t> src(fb.width() * fb.height());
            std::copy(fb.colorData(), fb.colorData() + src.size(),
                      src.begin());
            static std::vector<float> dep(fb.width() * fb.height());
            std::copy(fb.depthData(), fb.depthData() + dep.size(),
                      dep.begin());
            const std::size_t w = fb.width(), h = fb.height();
            for(std::size_t y = 1; y + 1 < h; y++){
                for(std::size_t x = 1; x + 1 < w; x++){
                    const float d = dep[y * w + x];
                    if(d <= 0.0f || d >= 1.0f){ continue; }
                    float occ = 0.0f;
                    for(int oy = -1; oy <= 1; oy++){
                        for(int ox = -1; ox <= 1; ox++){
                            const float dn =
                                dep[(y + oy) * w + (x + ox)];
                            if(dn > d + 0.008f){ occ += 0.14f; }
                        }
                    }
                    occ = std::min(occ, 0.65f);
                    if(occ <= 0.01f){ continue; }
                    const uint32_t c = src[y * w + x];
                    const uint32_t nr = static_cast<uint32_t>(
                        ((c >> 16) & 0xFF) * (1.0f - occ));
                    const uint32_t ng = static_cast<uint32_t>(
                        ((c >> 8) & 0xFF) * (1.0f - occ));
                    const uint32_t nb = static_cast<uint32_t>(
                        (c & 0xFF) * (1.0f - occ));
                    fb.setPixel(x, y,
                        0xFF000000u | (nr << 16) | (ng << 8) | nb, -2.0f);
                }
            }
        }
    }
    void drawUi(Application &) override {
        ImGui::Checkbox("SSAO", &m_ssaoOn);
        ImGui::Text("room box + nanosuit, bluish light");
    }
    const char *name() const override { return "SSAO Contact Shadows"; }
    const char *group() const override { return "LightAdv"; }
private:
    bool m_modelLoaded{false};
    Object4D m_model{};
    Texture m_gray{};
};

}
