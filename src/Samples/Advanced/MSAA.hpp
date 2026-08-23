#pragma once

#include "../SceneUtil.hpp"
#include "Render/PostProcess.hpp"

#include <cmath>

namespace SGE::Samples {

class MSAAScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(0xFF101018u);
        const double ang = app.angle();
        auto drawScene = [&](FrameBuffer &target){
            target.clearDepth();
            LightingRig rig{};
            rig.ambient = 0.25f;
            Rasterizer srz{target};
            SGE::Render::Camera mc{};
            mc.position = Vector3DBase<double>{0, 1.2, -3.2};
            auto vp = SGE::Math::perspective(M_PI / 3,
                static_cast<double>(target.width()) / target.height(), 0.1, 50.0)
                .mul(mc.viewMatrix());
            Object4D cube{};
            std::snprintf(cube.name, sizeof(cube.name), "%s", "c");
            const double s = 0.7;
            Point4D v[8] = {{-s,-s,-s,1},{s,-s,-s,1},{s,s,-s,1},{-s,s,-s,1},
                            {-s,-s,s,1},{s,-s,s,1},{s,s,s,1},{-s,s,s,1}};
            for(int i = 0; i < 8; i++){ cube.vlistLocal[i] = v[i]; }
            struct F{ int a,b,c; };
            const F fs[12] = {{0,3,2},{0,2,1},{4,5,6},{4,6,7},{0,1,5},{0,5,4},
                              {3,7,6},{3,6,2},{1,2,6},{1,6,5},{0,4,7},{0,7,3}};
            cube.numVertices = 8;
            cube.numPolys = 12;
            for(int i = 0; i < 12; i++){
                cube.plist[i].vlist[0] = v[fs[i].a];
                cube.plist[i].vlist[1] = v[fs[i].b];
                cube.plist[i].vlist[2] = v[fs[i].c];
                cube.plist[i].color = Color32{(i % 3) * 70 + 60,
                    (i % 4) * 55 + 40, (i % 5) * 45 + 30, 255};
            }
            auto cm = SGE::Math::translation(0.0, 0.9, 1.0)
                .mul(SGE::Math::rotationY(ang)).mul(SGE::Math::rotationX(0.5));
            auto cnrm = SGE::Math::normalMatrix(cm);
            auto tris = Pipeline::projectObject(cube, cm, vp, cnrm,
                static_cast<int>(target.width()),
                static_cast<int>(target.height()));
            for(auto &t : tris){ srz.drawTriangleSolid(t.v[0], t.v[1], t.v[2]); }
        };
        if(m_ssaaEnabled){
            static FrameBuffer big{1600, 1200};
            drawScene(big);
            // box downsample 2x
            const auto *src = big.colorData();
            for(int y = 0; y < 600; y++){
                for(int x = 0; x < 800; x++){
                    uint32_t acc[3]{};
                    for(int dy = 0; dy < 2; dy++){
                        for(int dx = 0; dx < 2; dx++){
                            const uint32_t c =
                                src[(y * 2 + dy) * 1600 + x * 2 + dx];
                            acc[0] += (c >> 16) & 0xFF;
                            acc[1] += (c >> 8) & 0xFF;
                            acc[2] += c & 0xFF;
                        }
                    }
                    const Color32 cc{static_cast<int32_t>(acc[0] / 4),
                        static_cast<int32_t>(acc[1] / 4),
                        static_cast<int32_t>(acc[2] / 4), 255};
                    fb.setPixel(x, y,
                        (static_cast<uint32_t>(cc.a) << 24)
                            | (static_cast<uint32_t>(cc.r) << 16)
                            | (static_cast<uint32_t>(cc.g) << 8)
                            | static_cast<uint32_t>(cc.b), -2.0f);
                }
            }
        }else{
            drawScene(fb);
        }
    }
    void drawUi(Application &) override {
        ImGui::Checkbox("2x Supersampling", &m_ssaaEnabled);
        ImGui::Text("Watch diagonal edges");
    }
    bool m_ssaaEnabled{true};
    const char *name() const override { return "Anti-aliasing (2x SSAA)"; }
    const char *group() const override { return "Advanced"; }
};

}
