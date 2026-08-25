#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"
#include "Render/PostProcess.hpp"

#include <cmath>

namespace SGE::Samples {

class BloomScene final : public IScene {
public:
    bool m_bloomEnabled{true};
    float m_threshold{0.75f};
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        static Texture wood = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/wood.png");
        static Texture bricks = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/bricks2.jpg");
        LightingRig rig{};
        rig.ambient = 0.1f;
        rig.specularStrength = 0.0f;
        // reference: 4 point lights with HDR-ish colors (scaled to LDR)
        struct RL{ double x,y,z; float r,g,b; };
        static const RL ls[4] = {
            {0, 0.5, 1.5 + 3.0, 5, 5, 5},
            {-4, 0.5, -3 + 3.0, 10, 0, 0},
            {3, 0.5, 1 + 3.0, 0, 0, 15},
            {-0.8, 2.4, -1 + 3.0, 0, 5, 0}};
        for(const auto &l : ls){
            PointLight p{};
            p.position = Vector3DBase<double>{l.x, l.y, l.z};
            p.color = ColorFlt{std::min(15.0f, l.r) / 5,
                               std::min(15.0f, l.g) / 5,
                               std::min(15.0f, l.b) / 5};
            p.range = 60.0;
            rig.point.push_back(p);
        }
        Rasterizer rz{fb};
        ShadingContext ctx{&rig, app.camera().position};
        const auto vp = refViewProj(app.camera());
        SGE::Render::TileRenderer tiled{fb};
        auto fpv = refPlane(Color32{255,255,255,255});
        auto fpm = SGE::Math::translation(0.0, -0.9, -2.0);
        auto fpnrm = SGE::Math::normalMatrix(fpm);
        tiled.drawTextured(Pipeline::projectObject(fpv, fpm,
            vp, fpnrm, g_renderW, g_renderH), bricks, &ctx);
        Object4D cube = unitCube(app);
        // reference GetCubePositions
        static const double cps[10][3] = {
            {0,0,0},{2,5,-15},{-1.5,-2.2,-2.5},{-3.8,-2,-12.3},
            {2.4,-0.4,-3.5},{-1.7,3,-7.5},{1.3,-2,-2.5},
            {1.5,2,-2.5},{1.5,0.2,-1.5},{-1.3,1,-1.5}};
        for(int i = 0; i < 10; i++){
            auto cm = SGE::Math::translation(cps[i][0],
                cps[i][1] + 2.0, cps[i][2] - 6.0)
                .mul(SGE::Math::rotationY(app.angle() * 0.2 * (i % 3)));
            auto cnrm = SGE::Math::normalMatrix(cm);
            tiled.drawTextured(Pipeline::projectObject(cube, cm,
                vp, cnrm, g_renderW, g_renderH), wood, &ctx);
        }
        {
            Object4D lampCube = unitCube(app);
            for(const auto &l : ls){
                auto lm = SGE::Math::translation(l.x, l.y, l.z)
                    .mul(SGE::Math::scale(0.25, 0.25, 0.25));
                auto ln = SGE::Math::normalMatrix(lm);
                auto lt = Pipeline::projectObject(lampCube, lm,
                    vp, ln, g_renderW, g_renderH);
                const Color32 lc{
                    static_cast<int32_t>(std::min(255.0f, l.r * 40)),
                    static_cast<int32_t>(std::min(255.0f, l.g * 40)),
                    static_cast<int32_t>(std::min(255.0f, l.b * 40)), 255};
                for(auto &t : lt){
                    t.v[0].color = lc; t.v[1].color = lc;
                    t.v[2].color = lc;
                    rz.drawTriangleSolid(t.v[0], t.v[1], t.v[2]);
                }
            }
        }
        if(m_bloomEnabled){
            // extract at half resolution then upsample-additive
            static FrameBuffer bright{400, 300};
            static FrameBuffer brightFull{static_cast<std::size_t>(g_renderW), static_cast<std::size_t>(g_renderH)};
            if(brightFull.width() != static_cast<std::size_t>(g_renderW)){ brightFull = FrameBuffer{static_cast<std::size_t>(g_renderW), static_cast<std::size_t>(g_renderH)}; }
            bright.clear();
            const auto *srcPx = fb.colorData();
            for(int y = 0; y < 300; y++){
                for(int x = 0; x < 400; x++){
                    const uint32_t c =
                        srcPx[(y * 2) * static_cast<std::size_t>(g_renderW) + x * 2];
                    const int lumR = (c >> 16) & 0xFF;
                    const int lumG = (c >> 8) & 0xFF;
                    const int lumB = c & 0xFF;
                    const double lum = 0.2126 * lumR
                        + 0.7152 * lumG + 0.0722 * lumB;
                    if(lum > m_threshold * 255.0){
                        const double k = (lum - m_threshold * 255.0)
                            / std::max(1e-3, lum);
                        bright.setPixel(x, y,
                            0xFF000000u
                            | (static_cast<uint32_t>(
                                lumR * k) << 16)
                            | (static_cast<uint32_t>(
                                lumG * k) << 8)
                            | static_cast<uint32_t>(
                                lumB * k), -2.0f);
                    }
                }
            }
            for(int i = 0; i < 3; i++){
                SGE::Render::GaussianBlur(bright, 5);
            }
            // upsample to full res with 2x2 box
            const auto *bp = bright.colorData();
            for(int y = 0; y < g_renderH; y++){
                for(int x = 0; x < g_renderW; x++){
                    brightFull.setPixel(x, y,
                        bp[(static_cast<std::size_t>(y) * bright.height() / g_renderH) * bright.width() + static_cast<std::size_t>(x) * bright.width() / g_renderW], -2.0f);
                }
            }
            SGE::Render::AdditiveBlend(fb, brightFull);
        }
        // reference Final.fs: 1-exp(-c*exposure) then gamma
        for(std::size_t y2 = 0; y2 < static_cast<std::size_t>(g_renderH); y2++){
            for(std::size_t x2 = 0; x2 < static_cast<std::size_t>(g_renderW); x2++){
                const uint32_t c =
                    fb.colorData()[y2 * static_cast<std::size_t>(g_renderW) + x2];
                auto tm = [](double ch){
                    return static_cast<uint32_t>(
                        std::pow(std::clamp(
                            1.0 - std::exp(-(ch / 255.0)), 0.0, 1.0),
                            1.0 / 2.2) * 255.0 + 0.5);
                };
                fb.setPixel(x2, y2,
                    0xFF000000u | (tm((c >> 16) & 0xFF) << 16)
                    | (tm((c >> 8) & 0xFF) << 8) | tm(c & 0xFF), -2.0f);
            }
        }
    }
    void drawUi(Application &) override {
        ImGui::Checkbox("Bloom", &m_bloomEnabled);
        ImGui::SliderFloat("Threshold", &m_threshold, 0.3f, 0.95f);
        ImGui::Text("wood cubes + 4 colored lamps");
    }
    const char *name() const override { return "Bloom (colored lamp array)"; }
    const char *group() const override { return "LightAdv"; }
};

}
