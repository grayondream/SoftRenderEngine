#include "Light.hpp"
#include <gtest/gtest.h>
#include <cstdint>

namespace{
constexpr uint32_t kGray(uint32_t v){
    return 0xFF000000u | (v << 16) | (v << 8) | v;
}
int chan(uint32_t px, int shift){ return static_cast<int>((px >> shift) & 0xFF); }
}

TEST(LightTest, AmbientOnly){
    LightingRig rig{};
    rig.ambient = 0.25f;
    const uint32_t out = shade(rig, Color32{200,200,200,255},
                               Vector3DBase<double>{0,0,-1}, Vector3DBase<double>{0,0,0},
                               Vector3DBase<double>{0,0,-5});
    EXPECT_EQ(chan(out,16), 50);
    EXPECT_EQ(out, 0xFF323232u);
}

TEST(LightTest, DirectionaLambert){
    LightingRig rig{};
    rig.ambient = 0.0f;
    rig.specularStrength = 0.0f;
    DirectionalLight dl{};
    dl.color = ColorFlt{1.0f, 1.0f, 1.0f};

    dl.direction = Vector3DBase<double>{0, 0, -1};
    rig.directional.clear();
    rig.directional.push_back(dl);
    uint32_t facing = shade(rig, Color32{128,128,128,255},
                            Vector3DBase<double>{0,0,-1}, Vector3DBase<double>{0,0,0},
                            Vector3DBase<double>{0,0,-5});

    dl.direction = Vector3DBase<double>{std::sqrt(3.0)/2, 0, -0.5};
    rig.directional.clear();
    rig.directional.push_back(dl);
    uint32_t sixty = shade(rig, Color32{128,128,128,255},
                           Vector3DBase<double>{0,0,-1}, Vector3DBase<double>{0,0,0},
                           Vector3DBase<double>{0,0,-5});

    dl.direction = Vector3DBase<double>{0, 0, 1};
    rig.directional.clear();
    rig.directional.push_back(dl);
    uint32_t back = shade(rig, Color32{128,128,128,255},
                          Vector3DBase<double>{0,0,-1}, Vector3DBase<double>{0,0,0},
                          Vector3DBase<double>{0,0,-5});

    EXPECT_NEAR(chan(facing,16), 128, 1);
    EXPECT_NEAR(chan(sixty,16), 64, 1);
    EXPECT_EQ(chan(back,16), 0);
    EXPECT_EQ(static_cast<int>((back >> 24) & 0xFF), 255);
}

TEST(LightTest, PointAttenuation){
    LightingRig rig{};
    rig.ambient = 0.0f;
    rig.specularStrength = 0.0f;
    PointLight pl{};
    pl.position = Vector3DBase<double>{0, 0, -5};
    pl.color = ColorFlt{1.0f, 1.0f, 1.0f};
    pl.range = 10.0;
    rig.point.push_back(pl);

    uint32_t nearPx = shade(rig, Color32{128,128,128,255},
                            Vector3DBase<double>{0,0,-1}, Vector3DBase<double>{0,0,0},
                            Vector3DBase<double>{0,0,-5});
    uint32_t farPx = shade(rig, Color32{128,128,128,255},
                           Vector3DBase<double>{0,0,-1}, Vector3DBase<double>{20,0,0},
                           Vector3DBase<double>{0,0,-5});

    EXPECT_EQ(chan(nearPx,16), 64);
    EXPECT_EQ(chan(farPx,16), 0);
}

TEST(LightTest, SpecularHalfVector){
    LightingRig rig{};
    rig.ambient = 0.0f;
    rig.specularStrength = 0.5f;
    DirectionalLight dl{};
    dl.direction = Vector3DBase<double>{0, 0, -1};
    dl.color = ColorFlt{1.0f, 1.0f, 1.0f};
    rig.directional.push_back(dl);

    uint32_t aligned = shade(rig, Color32{128,128,128,255},
                             Vector3DBase<double>{0,0,-1}, Vector3DBase<double>{0,0,0},
                             Vector3DBase<double>{0,0,-5});
    EXPECT_EQ(chan(aligned,16), 255);

    uint32_t offset = shade(rig, Color32{128,128,128,255},
                            Vector3DBase<double>{0,0,-1}, Vector3DBase<double>{0,0,0},
                            Vector3DBase<double>{5,0,-5});
    EXPECT_NEAR(chan(offset,16), 138, 2);
    EXPECT_NEAR(chan(offset,8), 138, 2);
    EXPECT_NEAR(chan(offset,0), 138, 2);
}

TEST(LightTest, ZeroRangePointLightIgnored){
    LightingRig rig{};
    rig.ambient = 0.0f;
    rig.specularStrength = 0.0f;
    PointLight pl{};
    pl.position = Vector3DBase<double>{0, 0, -5};
    pl.color = ColorFlt{1.0f, 1.0f, 1.0f};
    pl.range = 0.0;
    rig.point.push_back(pl);

    const uint32_t out = shade(rig, Color32{128,128,128,255},
                               Vector3DBase<double>{0,0,-1}, Vector3DBase<double>{0,0,0},
                               Vector3DBase<double>{0,0,-5});
    EXPECT_EQ(out, 0xFF000000u);
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
