#pragma once

#include "../SceneUtil.hpp"
#include "Render/PostProcess.hpp"

#include <cmath>

namespace SGE::Samples {

class IrradianceScene final : public IScene {
public:
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
    }

    void render(Application &app) override {
        static SGE::Render::HDRImage env = []{
            return SGE::Render::ImageLoader::loadHdr("assets/textures/newport_loft.hdr");
        }();
        static std::vector<SGE::Render::HDRImage> specMips = []{
            const auto e2 = SGE::Render::ImageLoader::loadHdr(
                "assets/textures/newport_loft.hdr");
            return SGE::Render::GetPrefilteredCached(
                e2, "assets/cache/ibl_spec_loft");
        }();
        // reference: cosine-weighted irradiance convolution (precomputed once)
        static SGE::Render::HDRImage irradiance = []{
            const SGE::Render::HDRImage e =
                SGE::Render::ImageLoader::loadHdr(
                    "assets/textures/newport_loft.hdr");
            return SGE::Render::GetIrradianceCached(
                e, "assets/cache/ibl_irradiance_loft.bin");
        }();
        auto &fb = app.framebuffer();
        SGE::Render::DrawEquirectSky(fb, app.camera(), env, m_exposure);
        // reference: 25-sphere grid (col,row in [-2,2]) at z=6, albedo red ramp
        LightingRig rig2{};
        rig2.ambient = 0.03f;
        const double lp[4][3] = {{-10,10,10},{10,10,10},{-10,-10,10},{10,-10,10}};
        for(int li = 0; li < 4; li++){
            PointLight p{};
            p.position = Vector3DBase<double>{lp[li][0], lp[li][1], lp[li][2]};
            p.range = 200.0;
            rig2.point.push_back(p);
        }
        Rasterizer rz{fb};
        ShadingContext ctx{&rig2, app.camera().position};
        ctx.iblEquirect = &irradiance;
        ctx.iblSpecMips = &specMips;
        Texture white(1, 1, std::vector<uint32_t>{0xFFFFFFFFu}.data());
        static Object4D protoBall = SGE::Render::MakeSphere(1.0, 24, 16);
        const auto vp = refViewProj(app.camera());
        for(int row = -2; row <= 2; row++){
            for(int col = -2; col <= 2; col++){
                PbrMaterial mat{};
                mat.metallic = app.pbrMetallic();
                mat.roughness = std::max(0.05f, app.pbrRoughness());
                ctx.pbr = &mat;
                Object4D ball = protoBall;
                const double sc = 0.4;
                for(int vi = 0; vi < static_cast<int>(ball.numVertices); vi++){
                    ball.vlistLocal[static_cast<std::size_t>(vi)].x *= sc;
                    ball.vlistLocal[static_cast<std::size_t>(vi)].y *= sc;
                    ball.vlistLocal[static_cast<std::size_t>(vi)].z *= sc;
                }
                auto bm = SGE::Math::translation(
                    static_cast<double>(col) * 1.0,
                    static_cast<double>(row) * 1.0, 6.0);
                auto bnrm = SGE::Math::normalMatrix(bm);
                auto bt = Pipeline::projectObject(ball, bm, vp, bnrm, g_renderW, g_renderH);
                for(auto &t : bt){
                    rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                            white, &ctx);
                }
            }
        }
    }
    void drawUi(Application &) override {
        ImGui::SliderFloat("Exposure", &m_exposure, 0.3f, 2.5f);
        ImGui::Text("Diffuse IBL from equirect env");
    }
    float m_exposure{1.0f};
    const char *name() const override { return "IBL Diffuse Irradiance (loft)"; }
    const char *group() const override { return "PBR"; }
};

}
