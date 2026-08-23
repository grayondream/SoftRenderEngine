#pragma once

#include "../SceneUtil.hpp"

#include <algorithm>

namespace SGE::Samples {

class ShowcasePbrScene final : public IScene {
public:
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        auto &cam = app.camera();
        auto rig = makeDefaultRig();

        fb.clear(0xFF000000u);
        Rasterizer rz{fb};

        SGE::Render::TileRenderer tiled{fb};
        const int cols = 7, rows = 5;
        for(int r = 0; r < rows; r++){
            for(int c = 0; c < cols; c++){
                PbrMaterial mat{};
                mat.baseColor = app.pbrBase();
                mat.metallic = std::clamp(
                    static_cast<float>(c) / (cols - 1) + app.pbrMetallic() - 0.5f,
                    0.0f, 1.0f);
                mat.roughness = std::clamp(
                    1.0f - static_cast<float>(r) / (rows - 1) + app.pbrRoughness() - 0.35f,
                    0.05f, 1.0f);
                ShadingContext pbrCtx{&rig, cam.position,
                                      nullptr, nullptr, nullptr, nullptr, &mat};
                Object4D ball = app.sphere();
                const double bx = -4.2 + c * 1.4;
                const double by = -0.6 + r * 1.15;
                ball.worldPos = Point4D{bx, by, 3.0, 1};
                auto bm = SGE::Math::translation(bx, by, 3.0);
                auto bnrm = SGE::Math::normalMatrix(bm);
                auto bt = Pipeline::projectObject(ball, bm,
                    defaultViewProj(app), bnrm, 800, 600);
                for(auto &t : bt){
                    rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                            app.checker(), &pbrCtx);
                }
            }
        }
    }
    void drawUi(Application &app) override {
        ImGui::SliderFloat("Metallic", &app.pbrMetallic(), 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &app.pbrRoughness(), 0.05f, 1.0f);
        ImGui::ColorEdit3("Base Color", app.pbrColorUi());
        app.pbrBase() = Color32{
            static_cast<int32_t>(app.pbrColorUi()[0] * 255.0f),
            static_cast<int32_t>(app.pbrColorUi()[1] * 255.0f),
            static_cast<int32_t>(app.pbrColorUi()[2] * 255.0f), 255};
    }
    const char *name() const override { return "Cook-Torrance PBR Sphere Array"; }
    const char *group() const override { return "Showcase"; }
};

}
