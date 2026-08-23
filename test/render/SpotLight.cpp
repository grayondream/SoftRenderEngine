#include "Light.hpp"
#include "SortUtil.hpp"
#include <gtest/gtest.h>
#include <vector>

namespace{
int chan(uint32_t px, int shift){ return static_cast<int>((px >> shift) & 0xFF); }
using Vec = Vector3DBase<double>;
}

TEST(SpotLightTest, ConeInsideLitOutsideDark){
    LightingRig rig{};
    rig.ambient = 0.0f;
    rig.specularStrength = 0.0f;
    SpotLight sp{};
    sp.position = Vec{0, 5, 0};
    sp.direction = Vec{0, -1, 0};
    sp.color = ColorFlt{1, 1, 1};
    sp.range = 20.0;
    sp.cutoffCos = 0.9;
    rig.spot.push_back(sp);

    const uint32_t inside = shade(rig, Color32{255,255,255,255},
        Vec{0,1,0}, Vec{0,2,0}, Vec{0,10,0});
    const uint32_t outside = shade(rig, Color32{255,255,255,255},
        Vec{0,1,0}, Vec{4,2,0}, Vec{0,10,0});
    EXPECT_GT(chan(inside, 16), 150);
    EXPECT_EQ(outside & 0x00FFFFFFu, 0u);
}

TEST(SpotLightTest, EdgeSofteningBetweenConeAndFull){
    LightingRig rig{};
    rig.ambient = 0.0f;
    rig.specularStrength = 0.0f;
    SpotLight sp{};
    sp.position = Vec{0, 5, 0};
    sp.direction = Vec{0, -1, 0};
    sp.color = ColorFlt{1, 1, 1};
    rig.spot.push_back(sp);

    const uint32_t center = shade(rig, Color32{128,128,128,255},
        Vec{0,1,0}, Vec{0,0,0}, Vec{0,10,0});
    const uint32_t midEdge = shade(rig, Color32{128,128,128,255},
        Vec{0,1,0}, Vec{-0.6,0,0}, Vec{0,10,0});
    EXPECT_GT(chan(midEdge, 16), 0);
    EXPECT_LT(chan(midEdge, 16), chan(center, 16));
}

TEST(SortUtilTest, FarToNearOrdering){
    std::vector<Vec> pos = {
        Vec{0,0,-2}, Vec{0,0,-8}, Vec{0,0,-5}};
    const auto order = SGE::Render::SortFarToNear(3,
        [&](int i){ return pos[static_cast<std::size_t>(i)]; },
        Vec{0,0,0});
    ASSERT_EQ(order.size(), 3u);
    EXPECT_EQ(order[0], 1);
    EXPECT_EQ(order[1], 2);
    EXPECT_EQ(order[2], 0);
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
