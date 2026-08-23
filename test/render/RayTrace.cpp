#include "RayTrace.hpp"
#include <algorithm>
#include <cmath>
#include <gtest/gtest.h>
#include <vector>

namespace{
using SGE::Render::RaySphere;
using SGE::Render::RayTriangle;
using SGE::Render::RayScene;
using SGE::Render::RayTraceOptions;
using ::LightingRig;
using ::DirectionalLight;
using ::ColorFlt;
using Vec = Vector3DBase<double>;

int chan(uint32_t px, int shift){ return static_cast<int>((px >> shift) & 0xFF); }

LightingRig KeyRig(){
    LightingRig rig{};
    rig.ambient = 0.25f;
    rig.specularStrength = 0.0f;
    DirectionalLight dl{};
    dl.color = ColorFlt{1, 1, 1};
    dl.direction = Vec{0, -1, 0};
    rig.directional.push_back(dl);
    return rig;
}
}

TEST(RaySphereTest, HitAndMiss){
    RaySphere s{};
    s.center = Vec{0, 0, 0};
    s.radius = 1.0;

    double d = 0;
    EXPECT_TRUE(SGE::Render::RayDetail::raySphereIntersect(
        s, Vec{0, 0, -5}, Vec{0, 0, 1}, d));
    EXPECT_NEAR(d, 4.0, 1e-9);

    EXPECT_FALSE(SGE::Render::RayDetail::raySphereIntersect(
        s, Vec{0, 0, -5}, Vec{0, 1, 0}, d));

    EXPECT_TRUE(SGE::Render::RayDetail::raySphereIntersect(
        s, Vec{0, 0, -5}, Vec{0, 0.999, 5}.normalize(), d));
}

TEST(RayTriangleTest, HitMissParallel){
    RayTriangle tri{};
    tri.a = Vec{-1, 0, 0};
    tri.b = Vec{1, 0, 0};
    tri.c = Vec{0, 1, 0};

    double d = 0;
    EXPECT_TRUE(SGE::Render::RayDetail::rayTriangleIntersect(
        tri, Vec{0, 0.25, -5}, Vec{0, 0, 1}, d));
    EXPECT_NEAR(d, 5.0, 1e-9);

    EXPECT_FALSE(SGE::Render::RayDetail::rayTriangleIntersect(
        tri, Vec{5, 5, -5}, Vec{0, 0, 1}, d));

    EXPECT_FALSE(SGE::Render::RayDetail::rayTriangleIntersect(
        tri, Vec{0, 0.25, -5}, Vec{1, 0, 0}, d));
}

TEST(RayShadowTest, BlockedFallsBackToAmbient){
    RayScene scene{};
    scene.spheres.push_back(RaySphere{Vec{0, 3, 0}, 1.0,
                                      Color32{200,200,200,255}, 0.0f});

    LightingRig rig = KeyRig();

    SGE::Render::Camera cam{};
    cam.position = Vec{0, 0, -6};
    cam.pitch = 0;

    FrameBuffer fb{8, 8};
    fb.clear();
    SGE::Render::RayTracer tracer{fb};
    RayTraceOptions opt{};
    tracer.render(scene, cam, rig, opt);

    int maxV = 0;
    for(std::size_t i = 0; i < 64u; i++){
        maxV = std::max(maxV, chan(fb.colorData()[i], 16));
    }
    EXPECT_EQ(maxV, 50);
}

TEST(RayMirrorTest, ReflectionPicksUpNeighborColor){
    RayScene scene{};
    RaySphere red{Vec{-2, 0, 0}, 1.0, Color32{255, 0, 0, 255}, 0.0f};
    RaySphere mirror{Vec{0, 0, 0}, 1.0, Color32{0, 0, 0, 255}, 1.0f};
    scene.spheres.push_back(red);
    scene.spheres.push_back(mirror);

    LightingRig rig{};
    rig.ambient = 1.0f;

    SGE::Render::Camera cam{};
    cam.position = Vec{0, 0, -6};
    cam.pitch = 0;

    FrameBuffer fb{16, 16};
    fb.clear();
    SGE::Render::RayTracer tracer{fb};
    RayTraceOptions opt{};
    opt.maxDepth = 3;
    tracer.render(scene, cam, rig, opt);

    bool sawRedTint = false;
    for(std::size_t i = 0; i < 256u; i++){
        const auto px = fb.colorData()[i];
        if(chan(px, 16) > 100 && chan(px, 0) < 60 && px != 0xFFFFFFFFu){
            sawRedTint = true;
        }
    }
    EXPECT_TRUE(sawRedTint);
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
