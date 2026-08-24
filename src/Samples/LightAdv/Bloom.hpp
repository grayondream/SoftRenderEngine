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
            p.color = ColorFlt{std::min(10.0f, l.r) / 8,
                               std::min(10.0f, l.g) / 8,
                               std::min(10.0f, l.b) / 8};
            p.range = 60.0;
            rig.point.push_back(p);
        }
        Rasterizer rz{fb};
        ShadingContext ctx{&rig, app.camera().position};
        const auto vp = refViewProj(app.camera());
        SGE::Render::TileRenderer tiled{fb};
        auto fpv = refPlane(Color32{255,255,255,255});
        auto fpm = SGE::Math::translation(0.0, -0.9, 3.0);
        auto fpnrm = SGE::Math::normalMatrix(fpm);
        tiled.drawTextured(Pipeline::projectObject(fpv, fpm,
            vp, fpnrm, 800, 600), bricks, &ctx);
        Object4D cube = unitCube(app);
        // reference GetCubePositions
        static const double cps[10][3] = {
            {0,0,0},{2,5,-15},{-1.5,-2.2,-2.5},{-3.8,-2,-12.3},
            {2.4,-0.4,-3.5},{-1.7,3,-7.5},{1.3,-2,-2.5},
            {1.5,2,-2.5},{1.5,0.2,-1.5},{-1.3,1,-1.5}};
        for(int i = 0; i < 10; i++){
            auto cm = SGE::Math::translation(cps[i][0],
                cps[i][1] + 2.0, cps[i][2] + 6.0)
                .mul(SGE::Math::rotationY(app.angle() * 0.2 * (i % 3)));
            auto cnrm = SGE::Math::normalMatrix(cm);
            tiled.drawTextured(Pipeline::projectObject(cube, cm,
                vp, cnrm, 800, 600), wood, &ctx);
        }
        for(const auto &l : ls){
            drawLamp(app, rz, Vector3DBase<double>{l.x, l.y, l.z}, 0.25);
        }
        if(m_bloomEnabled){
            static FrameBuffer bright{800, 600};
            SGE::Render::ExtractBright(fb, bright, m_threshold);
            for(int i = 0; i < 5; i++){
                SGE::Render::GaussianBlur(bright, 6);
            }
            SGE::Render::AdditiveBlend(fb, bright);
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
