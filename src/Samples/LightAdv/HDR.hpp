#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"
#include "Render/PostProcess.hpp"

#include <cmath>

namespace SGE::Samples {

class HDRTonemapScene final : public IScene {
public:
    float m_exposure{0.5f};
    void setup(Application &app) override {
        resetCamera(app, 0.0, 1.6, 3.0);
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        static Texture wood = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/wood.png");
        LightingRig rig{};
        rig.ambient = 0.0f;
        rig.specularStrength = 0.0f;
        // reference: 4 point lights, diffuse only
        struct RL{ double x,y,z; float r,g,b; };
        static const RL ls[4] = {
            {0, 0, 27.5, 200, 200, 200},
            {-1.4, -1.9, 9.0 - 14.0 + 8.0, 26, 0, 0},
            {0, -1.8, 12.0, 0, 0, 51},
            {0.8, -1.7, 16.0, 0, 26, 0}};
        for(const auto &l : ls){
            PointLight p{};
            p.position = Vector3DBase<double>{l.x * 2.5,
                l.y * 2.5 + 1.6, l.z};
            p.color = ColorFlt{
                std::min(10.0f, l.r) / 8,
                std::min(10.0f, l.g) / 8,
                std::min(10.0f, l.b) / 8};
            p.range = 300.0;
            rig.point.push_back(p);
        }
        Rasterizer rz{fb};
        ShadingContext ctx{&rig, app.camera().position};
        // corridor: inverted cube interior T(0,y,z) S(2.5,2.5,27.5)
        Object4D room{};
        std::snprintf(room.name, sizeof(room.name), "%s", "corridor");
        {
            double s2 = 0.5;
            Point4D v[8] = {{-s2,-s2,-s2,1},{s2,-s2,-s2,1},{s2,s2,-s2,1},
                            {-s2,s2,-s2,1},{-s2,-s2,s2,1},{s2,-s2,s2,1},
                            {s2,s2,s2,1},{-s2,s2,s2,1}};
            for(int i = 0; i < 8; i++){ room.vlistLocal[i] = v[i]; }
            struct F{ int a,b,c; };
            const F fs[12] = {{0,3,2},{0,2,1},{4,5,6},{4,6,7},{0,1,5},{0,5,4},
                              {3,7,6},{3,6,2},{1,2,6},{1,6,5},{0,4,7},{0,7,3}};
            room.numVertices = 8;
            room.numPolys = 12;
            for(int i = 0; i < 12; i++){
                for(int k = 0; k < 3; k++){
                    const int vi = (k == 0 ? fs[i].a :
                                    k == 1 ? fs[i].c : fs[i].b);
                    room.plist[static_cast<std::size_t>(i)].vlist[k] =
                        v[vi];
                }
                room.plist[static_cast<std::size_t>(i)].uvlist[0] = {0,0};
                room.plist[static_cast<std::size_t>(i)].uvlist[1] = {8,0};
                room.plist[static_cast<std::size_t>(i)].uvlist[2] = {8,8};
                room.plist[static_cast<std::size_t>(i)].color =
                    Color32{255,255,255,255};
            }
        }
        auto rm = SGE::Math::translation(0.0, 1.6, 14.0)
            .mul(SGE::Math::scale(2.5, 2.5, 27.5));
        auto rnrm = SGE::Math::normalMatrix(rm);
        auto cam = refCamera(0, 1.6, 3.0);
        auto rt = Pipeline::projectObject(room, rm,
            refViewProj(cam), rnrm, 800, 600);
        for(auto &t : rt){
            rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                    wood, &ctx);
        }
        // reference tonemap: 1 - exp(-c * exposure), then gamma 2.2
        static FrameBuffer tmp{800, 600};
        tonemapPass(fb, tmp, m_exposure);
    }
    void drawUi(Application &) override {
        ImGui::SliderFloat("Exposure", &m_exposure, 0.1f, 2.0f);
        ImGui::Text("dark wood corridor");
    }
    const char *name() const override { return "HDR Corridor Tonemapping"; }
    const char *group() const override { return "LightAdv"; }
private:
    static void tonemapPass(FrameBuffer &fb, const FrameBuffer &src,
                            float e){
        // 4096-entry exposure+gamma LUT replaces per-pixel exp/pow
        static float lut[4096];
        static float lutE = -1.0f;
        if(std::abs(lutE - e) > 1e-4f){
            for(int i = 0; i < 4096; i++){
                const double ch = i / 4095.0;
                lut[i] = static_cast<float>(
                    std::pow(std::clamp(
                        1.0 - std::exp(-ch * 6.0 * e), 0.0, 1.0),
                        1.0 / 2.2) * 255.0 + 0.5);
            }
            lutE = e;
        }
        const auto *px = src.colorData();
        const std::size_t w = src.width(), h = src.height();
        for(std::size_t y = 0; y < h; y++){
            for(std::size_t x = 0; x < w; x++){
                const uint32_t c = px[y * w + x];
                fb.setPixel(x, y,
                    0xFF000000u | (static_cast<uint32_t>(
                        lut[std::min(4095u,
                            ((c >> 16) & 0xFFu) * 16u)]) << 16)
                    | (static_cast<uint32_t>(
                        lut[std::min(4095u,
                            ((c >> 8) & 0xFFu) * 16u)]) << 8)
                    | static_cast<uint32_t>(lut[std::min(4095u,
                        (c & 0xFFu) * 16u)]), -2.0f);
            }
        }
    }
};

}
