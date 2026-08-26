#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"
#include "Render/PostProcess.hpp"

#include <cmath>
#include <vector>

namespace SGE::Samples {

class DepthTestScene final : public IScene {
public:
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
    }
public:
    bool m_depthView{true};
    void render(Application &app) override {
        static const Object4D cubeProto = []{
            Object4D c{};
            std::snprintf(c.name, sizeof(c.name), "%s", "c");
            double s = 0.5;
            Point4D v[8] = {{-s,-s,-s,1},{s,-s,-s,1},{s,s,-s,1},{-s,s,-s,1},
                            {-s,-s,s,1},{s,-s,s,1},{s,s,s,1},{-s,s,s,1}};
            for(int i = 0; i < 8; i++){ c.vlistLocal[i] = v[i]; }
            struct F{ int a,b,c; };
            const F fs[12] = {{0,3,2},{0,2,1},{4,5,6},{4,6,7},{0,1,5},{0,5,4},
                              {3,7,6},{3,6,2},{1,2,6},{1,6,5},{0,4,7},{0,7,3}};
            c.numVertices = 8;
            c.numPolys = 12;
            for(int i = 0; i < 12; i++){
                // outward-facing winding (sync with Application::MakeCube)
                for(int k = 0; k < 3; k++){
                    c.plist[i].vlist[k] = v[(k == 0 ? fs[i].a :
                                             k == 1 ? fs[i].c : fs[i].b)];
                }
                c.plist[i].uvlist[0] = {0,0};
                c.plist[i].uvlist[1] = {1,1};
                c.plist[i].uvlist[2] = {1,0};
            }
            return c;
        }();
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        static Texture marble = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/marble.jpg");
        LightingRig rig{};
        rig.ambient = 0.9f;
        ShadingContext ctx{&rig, app.camera().position};
        const auto vp = refViewProj(app.camera());
        SGE::Render::TileRenderer tiled{fb};
        for(int gx = -2; gx <= 1; gx++){
            for(int gy = -2; gy <= 1; gy++){
                for(int gz = -2; gz <= 1; gz++){
                    auto cm = SGE::Math::translation(
                        gx * 1.1, gy * 1.1 - 2.0, gz * 1.1 - 5.0)
                        .mul(SGE::Math::rotationY(0.3));
                    auto cnrm = SGE::Math::normalMatrix(cm);
                    auto ct = Pipeline::projectObject(cubeProto, cm,
                        vp, cnrm, g_renderW, g_renderH);
                    tiled.drawTextured(ct, marble, &ctx);
                }
            }
        }

    }
    void drawUi(Application &) override {
        ImGui::Begin("OpenGL");
        ImGui::End();
    }
    const char *name() const override { return "Depth Test Grid"; }
    const char *group() const override { return "Advanced"; }
};

}
