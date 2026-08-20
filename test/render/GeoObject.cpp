#include "Log.hpp"
#include "Object4D.hpp"
#include "Color.hpp"
#include "UUID.hpp"
#include <gtest/gtest.h>

TEST(Object4DTest, DefaultConstructible){
    Object4D obj{};
    EXPECT_EQ(obj.numVertices, 0);
    EXPECT_EQ(obj.numPolys, 0);
}

TEST(Object4DTest, PolyDefaultConstructible){
    PolyF4D poly{};
    EXPECT_EQ(poly.color.r, 0);
    EXPECT_EQ(poly.color.g, 0);
    EXPECT_EQ(poly.color.b, 0);
    EXPECT_EQ(poly.color.a, 0);
}

TEST(UUIDTest, ToStringNotEmpty){
    UUID u{};
    auto s = to_string(u);
    EXPECT_EQ(s.size(), 36);
}

TEST(UUIDTest, EqualityAndCopy){
    UUID a{};
    UUID b(a);
    EXPECT_TRUE(a == b);
    a.clear();
    EXPECT_FALSE(a == b);
}

TEST(ColorTest, Constructor){
    Color32 c(1, 2, 3, 4);
    EXPECT_EQ(c.r, 1);
    EXPECT_EQ(c.g, 2);
    EXPECT_EQ(c.b, 3);
    EXPECT_EQ(c.a, 4);
}

int main(int argc, char **argv){
    LOGI("Start render geoobject test");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}