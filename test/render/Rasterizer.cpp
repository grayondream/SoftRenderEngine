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

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
