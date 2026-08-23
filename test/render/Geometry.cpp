#include "Primitives.hpp"
#include <gtest/gtest.h>
#include <cmath>

using namespace SGE::Render;

namespace{
int CountDegenerate(const Object4D &o){
    int deg = 0;
    for(int i = 0; i < o.numPolys; i++){
        const auto &p = o.plist[i];
        const auto e1 = Vector3DBase<double>{p.vlist[1].x-p.vlist[0].x,
            p.vlist[1].y-p.vlist[0].y, p.vlist[1].z-p.vlist[0].z};
        const auto e2 = Vector3DBase<double>{p.vlist[2].x-p.vlist[0].x,
            p.vlist[2].y-p.vlist[0].y, p.vlist[2].z-p.vlist[0].z};
        if(e1.mul(e2).length() < 1e-9) deg++;
    }
    return deg;
}
}

TEST(GeometryTest, SphereTopologyAndNormals){
    auto s = MakeSphere(1.5);
    EXPECT_EQ(s.numVertices, 17 * 25);
    EXPECT_EQ(s.numPolys, 16 * 24 * 2 - 2 * 24);
    for(int i = 0; i < s.numVertices; i++){
        const double len = std::sqrt(s.vlistLocal[i].x*s.vlistLocal[i].x +
            s.vlistLocal[i].y*s.vlistLocal[i].y + s.vlistLocal[i].z*s.vlistLocal[i].z);
        EXPECT_NEAR(len, 1.5, 1e-9);
    }
    EXPECT_EQ(CountDegenerate(s), 0);
}

TEST(GeometryTest, TorusCylinderConeCounts){
    auto t = MakeTorus(2.0, 0.5);
    EXPECT_EQ(t.numVertices, 33 * 17);
    EXPECT_EQ(t.numPolys, 32 * 16 * 2);

    auto c = MakeCylinder(1.0, 2.0);
    EXPECT_EQ(c.numVertices, 25 * 2);
    EXPECT_EQ(c.numPolys, 24 * 2);

    auto k = MakeCone(1.0, 2.0);
    EXPECT_EQ(k.numVertices, 1 + 25);
    EXPECT_EQ(k.numPolys, 24);
    EXPECT_EQ(CountDegenerate(k), 0);
}

TEST(GeometryTest, TeapotScaleAndWelding){
    auto tp = MakeTeapot();
    EXPECT_EQ(tp.numVertices, 32 * 25);
    EXPECT_GE(tp.numPolys, 32 * 16 * 2 - 64);
    EXPECT_LE(tp.numVertices, kObject4vListLen);
    EXPECT_LE(tp.numPolys, kPolyListLen);
    double maxY = -1e9, minY = 1e9;
    for(int i = 0; i < tp.numVertices; i++){
        maxY = std::max(maxY, tp.vlistLocal[i].y);
        minY = std::min(minY, tp.vlistLocal[i].y);
    }
    EXPECT_GT(maxY, 0.5);
    EXPECT_LT(minY, 0.3);
    EXPECT_EQ(CountDegenerate(tp), 0);
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
