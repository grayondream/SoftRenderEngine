#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>

namespace SGE::Samples {

class SimpleTextureScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        Rasterizer rz{fb};
        static Texture dog = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/dog.jpg");
        // fullscreen-center quad via NDC rect with flipped V (image loaded top-down)
        Object4D plane{};
        std::snprintf(plane.name, sizeof(plane.name), "%s", "quad");
        Point4D pv[4] = {{-0.475,-0.633,0,1},{0.475,-0.633,0,1},
                         {0.475,0.633,0,1},{-0.475,0.633,0,1}};
        for(int i = 0; i < 4; i++){ plane.vlistLocal[i] = pv[i]; }
        plane.numVertices = 4;
        plane.numPolys = 2;
        const int idx[2][3] = {{0,1,2},{0,2,3}};
        const UV2D uvq[4] = {{0,1},{1,1},{1,0},{0,0}};
        for(int k = 0; k < 2; k++){
            for(int m = 0; m < 3; m++){
                plane.plist[k].vlist[m] = pv[idx[k][m]];
                plane.plist[k].uvlist[m] = uvq[idx[k][m]];
                plane.plist[k].nlist[m] = Vector3DBase<double>{0, 0, -1};
            }
            plane.plist[k].color = Color32{255,255,255,255};
        }
        auto pm = SGE::Math::translation(0.0, 0.0, 0.0);
        auto pt = Pipeline::projectObject(plane, pm,
            SGE::Math::perspective(M_PI / 3, static_cast<double>(g_renderW) / g_renderH, 0.01, 10.0)
                .mul(refCamera().viewMatrix()),
            SGE::Math::normalMatrix(pm), g_renderW, g_renderH);
        for(auto &t : pt){
            rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                    dog, nullptr, TextureFilter::Bilinear,
                                    TextureWrap::Clamp);
        }
    }
    void drawUi(Application &) override {
        ImGui::Text("dog.jpg centered, unlit");
    }
    const char *name() const override { return "Simple Texture"; }
    const char *group() const override { return "Base"; }
};

}
