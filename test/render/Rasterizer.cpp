#include "Rasterizer.hpp"
#include <gtest/gtest.h>

namespace{
int countPixels(FrameBuffer &fb, uint32_t v){
    int n = 0;
    for(std::size_t i = 0; i < fb.width()*fb.height(); i++)
        if(fb.colorData()[i] == v) n++;
    return n;
}
bool hasPixel(FrameBuffer &fb, std::size_t x, std::size_t y, uint32_t v){
    return fb.colorData()[y*fb.width()+x] == v;
}
ScreenVertex V(double x, double y){
    ScreenVertex v{};
    v.x = x; v.y = y; v.z = 0; v.w = 1;
    v.color = Color32{255, 0, 0, 255};
    return v;
}
uint32_t packRed(){
    return (255u << 24) | (255u << 16);
}
}

TEST(RasterLineTest, Horizontal){
    FrameBuffer fb{10, 10};
    Rasterizer rz{fb};
    rz.drawLine(V(2, 5), V(7, 5));
    EXPECT_EQ(countPixels(fb, packRed()), 6);
    EXPECT_TRUE(hasPixel(fb, 2, 5, packRed()));
    EXPECT_TRUE(hasPixel(fb, 7, 5, packRed()));
}

TEST(RasterLineTest, Vertical){
    FrameBuffer fb{10, 10};
    Rasterizer rz{fb};
    rz.drawLine(V(3, 1), V(3, 6));
    EXPECT_EQ(countPixels(fb, packRed()), 6);
}

TEST(RasterLineTest, Diagonal){
    FrameBuffer fb{10, 10};
    Rasterizer rz{fb};
    rz.drawLine(V(1, 1), V(5, 5));
    EXPECT_EQ(countPixels(fb, packRed()), 5);
    EXPECT_TRUE(hasPixel(fb, 1, 1, packRed()));
    EXPECT_TRUE(hasPixel(fb, 5, 5, packRed()));
}

TEST(RasterWireTest, SquarePerimeterOnly){
    FrameBuffer fb{10, 10};
    Rasterizer rz{fb};
    rz.drawTriangleWireframe(V(2,2), V(8,2), V(2,8));
    EXPECT_TRUE(hasPixel(fb, 2, 2, packRed()));
    EXPECT_TRUE(hasPixel(fb, 8, 2, packRed()));
    EXPECT_TRUE(hasPixel(fb, 2, 8, packRed()));
    EXPECT_FALSE(hasPixel(fb, 4, 4, packRed()));
}

TEST(RasterSolidTest, DegenerateWritesNothing){
    FrameBuffer fb{10, 10};
    Rasterizer rz{fb};
    rz.drawTriangleSolid(V(3,3), V(3,3), V(3,3));
    EXPECT_EQ(countPixels(fb, packRed()), 0);
}

TEST(RasterSolidTest, SmallTriangleExactPixels){
    FrameBuffer fb{10, 10};
    Rasterizer rz{fb};
    // (2,2),(6,2),(2,6): hypotenuse x+y=8 passes exactly through centers
    // of (4,3),(3,4),(2,5); top-left rule assigns them to this triangle
    rz.drawTriangleSolid(V(2,2), V(6,2), V(2,6));
    EXPECT_TRUE(hasPixel(fb, 2, 2, packRed()));
    EXPECT_TRUE(hasPixel(fb, 4, 2, packRed()));
    EXPECT_TRUE(hasPixel(fb, 2, 5, packRed()));
    EXPECT_TRUE(hasPixel(fb, 3, 3, packRed()));
    EXPECT_TRUE(hasPixel(fb, 4, 3, packRed()));   // on hypotenuse: top-left owned
    EXPECT_TRUE(hasPixel(fb, 3, 4, packRed()));   // on hypotenuse
    EXPECT_FALSE(hasPixel(fb, 5, 5, packRed()));  // beyond hypotenuse
    EXPECT_FALSE(hasPixel(fb, 6, 6, packRed()));
}

TEST(RasterSolidTest, SharedEdgeNoOverlap){
    auto tri1 = [](Rasterizer &rz){ rz.drawTriangleSolid(V(2,2), V(8,2), V(2,8)); };
    auto tri2 = [](Rasterizer &rz){ rz.drawTriangleSolid(V(8,2), V(8,8), V(2,8)); };

    FrameBuffer f1{12,12}; Rasterizer r1{f1}; tri1(r1);
    FrameBuffer f2{12,12}; Rasterizer r2{f2}; tri2(r2);
    int expect = countPixels(f1, packRed()) + countPixels(f2, packRed());
    ASSERT_GT(expect, 0);

    FrameBuffer fb{12,12}; Rasterizer rz{fb};
    tri1(rz); tri2(rz);
    EXPECT_EQ(countPixels(fb, packRed()), expect);
}

TEST(RasterSolidTest, PerspectiveCorrectDepth){
    FrameBuffer fb{8, 8};
    Rasterizer rz{fb};
    ScreenVertex a = V(2, 6), b = V(6, 6), c = V(4, 1);
    a.w = 1; b.w = 1; c.w = 4;
    a.z = -0.5f; b.z = -0.5f; c.z = -0.9f;
    rz.drawTriangleSolid(a, b, c);
    float d = fb.depthData()[5*8+4];   // pixel (4,5), interior row
    ASSERT_LT(d, 0.0f);
    EXPECT_NEAR(d, -0.51f, 0.05f);     // perspective-correct pull toward vertex z
}

TEST(RasterSolidTest, FartherTriangleFullyRejected){
    FrameBuffer fb{8, 8};
    Rasterizer rz{fb};
    auto tri = [&](float z){
        ScreenVertex t=V(2,2), u=V(6,2), s=V(2,6);
        t.z=u.z=s.z=z;
        rz.drawTriangleSolid(t,u,s);
    };
    tri(-0.2f);   // near, red
    ScreenVertex t=V(3,3), u=V(5,3), s=V(3,5);
    t.z=u.z=s.z=0.5f;   // farther in NDC (-1 near .. +1 far)
    t.color=u.color=s.color=Color32{255,0,255,255};
    rz.drawTriangleSolid(t,u,s);
    uint32_t magenta = (255u<<24) | (255u<<16);  // placeholder replaced below
    magenta = PackBGRA(Color32{255,0,255,255});
    EXPECT_EQ(countPixels(fb, magenta), 0);
    EXPECT_TRUE(hasPixel(fb, 3, 3, packRed()));
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
