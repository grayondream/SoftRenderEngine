#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>
#include <vector>

namespace SGE::Samples {

class BlendScene final : public IScene {
public:
public:
    int m_winCount{5};
    void setup(Application &) override {
        m_winQuad = makePlane(0.5, 0.0,
            Vector3DBase<double>{0, 0, -1}, Color32{255,255,255,255});
        for(int i = 0; i < static_cast<int>(m_winQuad.numPolys); i++){
            m_winQuad.plist[i].uvlist[0] = {0,1};
            m_winQuad.plist[i].uvlist[1] = {1,1};
            m_winQuad.plist[i].uvlist[2] = {1,0};
        }
    }
private:
    Object4D m_winQuad{};
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        static Texture marble = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/marble.jpg");
        static Texture metal = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/metal.png");
        static Texture window = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/window.png");
        Rasterizer rz{fb};
        LightingRig rig{};
        rig.ambient = 0.9f;
        rig.directional.clear();
        ShadingContext ctx{&rig, app.camera().position};
        const auto vp = refViewProj(app.camera());
        SGE::Render::TileRenderer tiled{fb};
        // opaque: floor + two marble cubes
        Object4D cube = unitCube(app);
        struct CPos{ double x, y, z; };
        const CPos cubes[2] = {{-1.0, 0.0, -1.0}, {2.0, 0.0, 0.0}};
        auto fpv = refPlane();
        auto fpm = SGE::Math::translation(0.0, 0.0, 0.0);
        auto fpnrm = SGE::Math::normalMatrix(fpm);
        tiled.drawTextured(Pipeline::projectObject(fpv, fpm,
            vp, fpnrm, 800, 600), metal, &ctx);
        for(const auto &c : cubes){
            auto cm = SGE::Math::translation(c.x, c.y + 0.5, c.z);
            auto cnrm = SGE::Math::normalMatrix(cm);
            tiled.drawTextured(Pipeline::projectObject(cube, cm,
                vp, cnrm, 800, 600), marble, &ctx);
        }
        // transparent windows far->near (reference positions)
        static const double wins[5][3] = {
            {-1.5, 0, -0.48}, {1.5, 0, 0.51}, {0, 0, 0.7},
            {-0.3, 0, -2.3}, {0.5, 0, -0.6}};
        struct Win{ double x, y, z; };
        std::vector<Win> ws;
        for(int i = 0; i < m_winCount; i++){
            ws.push_back(Win{wins[i][0], wins[i][1] + 0.5, wins[i][2]});
        }
        const auto eye = app.camera().position;
        std::sort(ws.begin(), ws.end(), [&](const Win &a, const Win &b){
            double da = (a.x - eye.x) * (a.x - eye.x)
                + (a.y - eye.y) * (a.y - eye.y)
                + (a.z - eye.z) * (a.z - eye.z);
            double db = (b.x - eye.x) * (b.x - eye.x)
                + (b.y - eye.y) * (b.y - eye.y)
                + (b.z - eye.z) * (b.z - eye.z);
            return da > db;
        });
        for(const auto &w : ws){
            auto wm = SGE::Math::translation(w.x, w.y, w.z);
            auto wnrm = SGE::Math::normalMatrix(wm);
            auto wt = Pipeline::projectObject(m_winQuad, wm,
                vp, wnrm, 800, 600);
            for(auto &t : wt){
                rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                        window, nullptr,
                                        TextureFilter::Bilinear,
                                        TextureWrap::Clamp);
            }
        }
    }
    void drawUi(Application &) override {
        ImGui::Text("marble cubes behind sorted glass windows");
    }
    const char *name() const override { return "Alpha Blend Windows"; }
    const char *group() const override { return "Advanced"; }
};

}
