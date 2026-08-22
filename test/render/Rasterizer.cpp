#include "Rasterizer.hpp"
#include "Texture.hpp"
#include "Light.hpp"
#include <cmath>
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

TEST(RasterTexturedTest, PerspectiveCorrectNotAffine){
    FrameBuffer fb(24, 24);
    Rasterizer rz{fb};

    uint32_t px[8];
    for(int j = 0; j < 8; j++){
        px[j] = 0xFF000000u | (static_cast<uint32_t>(j * 30) << 16);
    }
    Texture tex(8, 1, px);

    Color32 white{255, 255, 255, 255};
    ScreenVertex a{}, b{}, c{};
    a.x = 2;  a.y = 2;  a.w = 2; a.u = 0; a.z = -2; a.color = white;
    b.x = 18; b.y = 2;  b.w = 2; b.u = 1; b.z = -2; b.color = white;
    c.x = 2;  c.y = 18; c.w = 1; c.u = 0; c.z = -1; c.color = white;

    rz.drawTriangleTextured(a, b, c, tex, nullptr, TextureFilter::Nearest, TextureWrap::Clamp);

    // 像素(6,6)采样点(6.5,6.5)：lambda=(0.4375, 0.28125, 0.28125)
    // 透视校正 u = (0.28125/2)/(0.4375/2 + 0.28125/2 + 0.28125/1) = 9/41 ≈ 0.2195 -> texel1 -> r=30
    // 仿射插值 u = 0.28125 -> texel2 -> r=60（错误实现会得此值）
    // 简报原常数 0xFF001E00 把 0x1E 写在绿色字节，与其构造行 (j*30)<<16 及注释 r=30 矛盾，
    // 按 texel1 原始存储值修正为 0xFF1E0000
    const uint32_t got = fb.colorData()[6 * 24 + 6];
    EXPECT_EQ(got, 0xFF1E0000u);
}

TEST(RasterTexturedTest, QuadDiagonalUvContinuity){
    FrameBuffer fb(14, 14);
    Rasterizer rz{fb};

    std::vector<uint32_t> px(200);
    for(int j = 0; j < 200; j++){
        px[j] = 0xFF000000u | (static_cast<uint32_t>(j) << 16);
    }
    Texture tex(200, 1, px.data());

    Color32 white{255, 255, 255, 255};
    ScreenVertex A{}, B{}, C{}, D{};
    A.x = 2;  A.y = 2;  A.w = 1; A.u = 0; A.v = 0; A.z = -1; A.color = white;
    B.x = 10; B.y = 2;  B.w = 1; B.u = 1; B.v = 0; B.z = -1; B.color = white;
    C.x = 10; C.y = 10; C.w = 1; C.u = 1; C.v = 1; C.z = -1; C.color = white;
    D.x = 2;  D.y = 10; D.w = 1; D.u = 0; D.v = 1; D.z = -1; D.color = white;

    rz.drawTriangleTextured(A, B, C, tex, nullptr, TextureFilter::Nearest, TextureWrap::Clamp);
    rz.drawTriangleTextured(A, C, D, tex, nullptr, TextureFilter::Nearest, TextureWrap::Clamp);

    // w=1 时透视校正退化为仿射：u(px) = ((px+0.5)-2)/8，r = floor(u*200)
    // 探针取 u*200=x.5 形式避开 floor 边界；三个探针分居对角线 AC 两侧
    auto expectR = [](int p){
        const double u = ((p + 0.5) - 2.0) / 8.0;
        return static_cast<int>(std::floor(u * 200.0));
    };
    const int probes[][2] = {{3, 5}, {7, 6}, {5, 8}};
    for(auto &pr : probes){
        const uint32_t got = fb.colorData()[pr[1] * 14 + pr[0]];
        EXPECT_EQ(static_cast<int>((got >> 16) & 0xFF), expectR(pr[0]))
            << "px=" << pr[0] << " py=" << pr[1];
    }
}

TEST(RasterLitTest, LitVsUnlitGradient){
    uint32_t gray[1] = {0xFF808080u};
    Texture tex(1, 1, gray);

    Color32 white{255, 255, 255, 255};
    ScreenVertex a{}, b{}, c{};
    a.x = 2;  a.y = 2;  a.z = -1; a.w = 1; a.color = white;
    a.nx = 0; a.ny = 0; a.nz = -1; a.wx = 2; a.wy = 2; a.wz = -1;
    b.x = 18; b.y = 2;  b.z = -1; b.w = 1; b.color = white;
    b.nx = 0; b.ny = 0; b.nz = -1; b.wx = 18; b.wy = 2; b.wz = -1;
    c.x = 2;  c.y = 18; c.z = -1; c.w = 1; c.color = white;
    c.nx = 0; c.ny = 0; c.nz = -1; c.wx = 2; c.wy = 18; c.wz = -1;

    FrameBuffer fbUnlit(24, 24);
    FrameBuffer fbAway(24, 24);
    FrameBuffer fbToward(24, 24);
    Rasterizer rzU{fbUnlit}, rzA{fbAway}, rzT{fbToward};

    rzU.drawTriangleTextured(a, b, c, tex);

    LightingRig rigAway{};
    rigAway.ambient = 0.15f;
    rigAway.specularStrength = 0.0f;
    DirectionalLight dlA{};
    dlA.direction = Vector3DBase<double>{0, 0, 1};
    dlA.color = ColorFlt{1.0f, 1.0f, 1.0f};
    rigAway.directional.push_back(dlA);
    ShadingContext ctxA{&rigAway, Vector3DBase<double>{0, 0, -5}};
    rzA.drawTriangleTextured(a, b, c, tex, &ctxA);

    LightingRig rigToward = rigAway;
    rigToward.directional.clear();
    DirectionalLight dlT{};
    dlT.direction = Vector3DBase<double>{0, 0, -1};
    dlT.color = ColorFlt{1.0f, 1.0f, 1.0f};
    rigToward.directional.push_back(dlT);
    ShadingContext ctxT{&rigToward, Vector3DBase<double>{0, 0, -5}};
    rzT.drawTriangleTextured(a, b, c, tex, &ctxT);

    const std::size_t probe = 6 * 24 + 6;
    EXPECT_EQ(fbUnlit.colorData()[probe], 0xFF808080u);
    EXPECT_EQ(fbAway.colorData()[probe], 0xFF131313u);
    EXPECT_EQ(fbToward.colorData()[probe], 0xFF939393u);
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
