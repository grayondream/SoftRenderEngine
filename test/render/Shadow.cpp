#include "Rasterizer.hpp"
#include "Shadow.hpp"
#include <gtest/gtest.h>
#include <cmath>

namespace{
int chan(uint32_t px, int shift){ return static_cast<int>((px >> shift) & 0xFF); }

void FillQuadDepth(Rasterizer &rz, float z){
    const ScreenVertex a{-30, -30, z, 1};
    const ScreenVertex b{30, -30, z, 1};
    const ScreenVertex c{30, 30, z, 1};
    const ScreenVertex d{-30, 30, z, 1};
    rz.drawTriangleDepth(a, b, c);
    rz.drawTriangleDepth(a, c, d);
}
}

TEST(ShadowVPTest, CenterMapsToNdcOrigin){
    const auto vp = SGE::Render::directionalLightVP(
        Vector3DBase<double>{0.3, -0.8, -0.5},
        Vector3DBase<double>{2, 1, 3}, 8.0);

    double x = vp[0][0][0][0]*2 + vp[0][0][0][1]*1 + vp[0][0][0][2]*3 + vp[0][0][0][3];
    double y = vp[0][0][1][0]*2 + vp[0][0][1][1]*1 + vp[0][0][1][2]*3 + vp[0][0][1][3];
    double w = vp[0][0][3][0]*2 + vp[0][0][3][1]*1 + vp[0][0][3][2]*3 + vp[0][0][3][3];
    EXPECT_NEAR(x / w, 0.0, 1e-9);
    EXPECT_NEAR(y / w, 0.0, 1e-9);
    EXPECT_NEAR(w, 1.0, 1e-9);
}

TEST(ShadowPassTest, DepthPassRecordsNearestSurface){
    FrameBuffer fb{16, 16};
    fb.clear();
    Rasterizer rz{fb};

    FillQuadDepth(rz, -0.5f);
    FillQuadDepth(rz, 0.5f);

    for(std::size_t i = 0; i < 256u; i++){
        EXPECT_FLOAT_EQ(fb.depthData()[i], -0.5f);
    }
}

TEST(ShadowTest, OccludedPixelFallsBackToAmbient){
    FrameBuffer smap{64, 64};
    smap.clear();
    {
        Rasterizer srz{smap};
        const ScreenVertex a{0, 0, -0.5f, 1};
        const ScreenVertex b{64, 0, -0.5f, 1};
        const ScreenVertex c{64, 64, -0.5f, 1};
        const ScreenVertex d{0, 64, -0.5f, 1};
        srz.drawTriangleDepth(a, b, c);
        srz.drawTriangleDepth(a, c, d);
    }

    const auto lvp = SGE::Render::directionalLightVP(
        Vector3DBase<double>{0, 0, -1}, Vector3DBase<double>{0, 0, 0}, 4.0);
    SGE::Render::ShadowData sd{&smap, lvp, 0.005};

    LightingRig rig{};
    rig.ambient = 0.25f;
    DirectionalLight dl{};
    dl.color = ColorFlt{1, 1, 1};
    dl.direction = Vector3DBase<double>{0, 0, -1};
    rig.directional.push_back(dl);

    Texture tex(1, 1, std::vector<uint32_t>{0xFF808080u}.data());
    ShadingContext ctx{&rig, Vector3DBase<double>{0, 0, -5}};

    FrameBuffer litFb{32, 32}, shFb{32, 32};
    litFb.clear(); shFb.clear();

    ScreenVertex tri[3] = {};
    tri[0] = {4, 31, -0.5f, 1};
    tri[1] = {27, 31, -0.5f, 1};
    tri[2] = {15, 6, -0.5f, 1};
    for(auto &v : tri){
        v.nx = 0; v.ny = 0; v.nz = -1;
        v.wx = 0; v.wy = 0; v.wz = 0;
        v.u = 0; v.v = 0;
    }
    ctx.shadow = nullptr;
    Rasterizer rLit{litFb};
    rLit.drawTriangleTextured(tri[0], tri[1], tri[2], tex, &ctx);

    ctx.shadow = &sd;
    Rasterizer rSh{shFb};
    rSh.drawTriangleTextured(tri[0], tri[1], tri[2], tex, &ctx);

    std::size_t probe = 15u * 32u + 15u;
    EXPECT_GT(chan(litFb.colorData()[probe], 16), chan(shFb.colorData()[probe], 16));
}

TEST(ShadowTest, NoShadowContextUnchanged){
    LightingRig rig{};
    rig.ambient = 0.25f;
    DirectionalLight dl{};
    dl.color = ColorFlt{1, 1, 1};
    dl.direction = Vector3DBase<double>{0, 0, -1};
    rig.directional.push_back(dl);

    const uint32_t base = shade(rig, Color32{200,200,200,255},
                                Vector3DBase<double>{0,0,-1},
                                Vector3DBase<double>{0,0,0},
                                Vector3DBase<double>{0,0,-5});
    const uint32_t half = shade(rig, Color32{200,200,200,255},
                                Vector3DBase<double>{0,0,-1},
                                Vector3DBase<double>{0,0,0},
                                Vector3DBase<double>{0,0,-5}, 0.5);
    const uint32_t zero = shade(rig, Color32{200,200,200,255},
                                Vector3DBase<double>{0,0,-1},
                                Vector3DBase<double>{0,0,0},
                                Vector3DBase<double>{0,0,-5}, 0.0);
    EXPECT_EQ(base, 0xFFFFFFFFu);
    EXPECT_LT(chan(half, 16), chan(base, 16));
    EXPECT_EQ(zero, 0xFF323232u);
}

TEST(ShadowTest, PcfSoftensShadowEdge){
    FrameBuffer smap{64, 64};
    smap.clear();
    {
        Rasterizer srz{smap};
        const ScreenVertex a{0, 32, -0.5f, 1};
        const ScreenVertex b{64, 32, -0.5f, 1};
        const ScreenVertex c{64, 64, -0.5f, 1};
        const ScreenVertex d{0, 64, -0.5f, 1};
        srz.drawTriangleDepth(a, b, c);
        srz.drawTriangleDepth(a, c, d);
    }

    const auto lvp = SGE::Render::directionalLightVP(
        Vector3DBase<double>{0, 0, -1}, Vector3DBase<double>{0, 0, 0}, 4.0);
    SGE::Render::ShadowData sd{&smap, lvp, 0.005};
    sd.pcfRadius = 1;

    LightingRig rig{};
    rig.ambient = 0.25f;
    DirectionalLight dl{};
    dl.color = ColorFlt{1, 1, 1};
    dl.direction = Vector3DBase<double>{0, 0, -1};
    rig.directional.push_back(dl);

    Texture tex(1, 1, std::vector<uint32_t>{0xFF808080u}.data());
    ShadingContext ctx{&rig, Vector3DBase<double>{0, 0, -5}, nullptr, &sd};

    FrameBuffer fb{32, 32};
    fb.clear();
    Rasterizer rz{fb};
    ScreenVertex tri[3] = {};
    tri[0] = {4, 31, -0.5f, 1};
    tri[1] = {27, 31, -0.5f, 1};
    tri[2] = {15, 6, -0.5f, 1};
    for(auto &v : tri){
        v.nx = 0; v.ny = 0; v.nz = -1;
        v.wx = 0; v.wy = 0; v.wz = 0;
        v.u = 0; v.v = 0;
    }
    rz.drawTriangleTextured(tri[0], tri[1], tri[2], tex, &ctx);

    std::size_t edgePixel = 15u * 32u + 15u;
    const int v = chan(fb.colorData()[edgePixel], 16);
    EXPECT_GT(v, 32);
    EXPECT_LT(v, 210);
}

TEST(ShadowTest, PointLightVPOccludes){
    FrameBuffer smap{64, 64};
    smap.clear();
    {
        Rasterizer srz{smap};
        const ScreenVertex a{0, 0, -0.5f, 1};
        const ScreenVertex b{64, 0, -0.5f, 1};
        const ScreenVertex c{64, 64, -0.5f, 1};
        const ScreenVertex d{0, 64, -0.5f, 1};
        srz.drawTriangleDepth(a, b, c);
        srz.drawTriangleDepth(a, c, d);
    }

    const auto lvp = SGE::Render::pointLightVP(
        Vector3DBase<double>{0, 0, -6}, Vector3DBase<double>{0, 0, 0},
        M_PI / 3, 1.0, 0.1, 50.0);
    SGE::Render::ShadowData sd{&smap, lvp, 0.005};

    LightingRig rig{};
    rig.ambient = 0.25f;
    DirectionalLight dl{};
    dl.color = ColorFlt{1, 1, 1};
    dl.direction = Vector3DBase<double>{0, 0, -1};
    rig.directional.push_back(dl);

    Texture tex(1, 1, std::vector<uint32_t>{0xFF808080u}.data());
    ShadingContext ctx{&rig, Vector3DBase<double>{0, 0, -5}, nullptr, &sd};

    FrameBuffer litFb{32, 32}, shFb{32, 32};
    litFb.clear(); shFb.clear();

    ScreenVertex tri[3] = {};
    tri[0] = {4, 31, -0.5f, 1};
    tri[1] = {27, 31, -0.5f, 1};
    tri[2] = {15, 6, -0.5f, 1};
    for(auto &v : tri){
        v.nx = 0; v.ny = 0; v.nz = -1;
        v.wx = 0; v.wy = 0; v.wz = 0;
        v.u = 0; v.v = 0;
    }
    ctx.shadow = nullptr;
    Rasterizer rLit{litFb};
    rLit.drawTriangleTextured(tri[0], tri[1], tri[2], tex, &ctx);

    ctx.shadow = &sd;
    Rasterizer rSh{shFb};
    rSh.drawTriangleTextured(tri[0], tri[1], tri[2], tex, &ctx);

    std::size_t probe = 15u * 32u + 15u;
    EXPECT_GT(chan(litFb.colorData()[probe], 16), chan(shFb.colorData()[probe], 16));
}

TEST(ShadowTest, CubeShadowOccludesOnAllAxes){
    FrameBuffer faceZpos{32, 32};
    faceZpos.clear();
    {
        Rasterizer srz{faceZpos};
        const ScreenVertex a{0, 0, -0.5f, 1};
        const ScreenVertex b{32, 0, -0.5f, 1};
        const ScreenVertex c{32, 32, -0.5f, 1};
        const ScreenVertex d{0, 32, -0.5f, 1};
        srz.drawTriangleDepth(a, b, c);
        srz.drawTriangleDepth(a, c, d);
    }

    SGE::Render::CubeShadowData cs{};
    cs.faces[4] = &faceZpos;
    cs.lightPos = Vector3DBase<double>{0, 0, 0};
    cs.bias = 0.005;

    LightingRig rig{};
    rig.ambient = 0.25f;
    DirectionalLight dl{};
    dl.color = ColorFlt{1, 1, 1};
    dl.direction = Vector3DBase<double>{0, 0, -1};
    rig.directional.push_back(dl);

    Texture tex(1, 1, std::vector<uint32_t>{0xFF808080u}.data());
    ShadingContext ctx{&rig, Vector3DBase<double>{0, 0, -5}, nullptr, nullptr, &cs};

    auto renderProbe = [&](double wz){
        FrameBuffer fb{32, 32};
        fb.clear();
        Rasterizer rz{fb};
        ScreenVertex tri[3] = {};
        tri[0] = {4, 31, -0.5f, 1};
        tri[1] = {27, 31, -0.5f, 1};
        tri[2] = {15, 6, -0.5f, 1};
        for(auto &v : tri){
            v.nx = 0; v.ny = 0; v.nz = -1;
            v.wx = 0; v.wy = 0; v.wz = wz;
            v.u = 0; v.v = 0;
        }
        rz.drawTriangleTextured(tri[0], tri[1], tri[2], tex, &ctx);
        return chan(fb.colorData()[15u * 32u + 15u], 16);
    };

    const int litOppositeFace = renderProbe(-3.0);
    const int shadowedOnFace = renderProbe(3.0);
    EXPECT_GT(litOppositeFace, 100);
    EXPECT_EQ(shadowedOnFace, 32);
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
