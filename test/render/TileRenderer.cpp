#include "TileRenderer.hpp"
#include <gtest/gtest.h>
#include <cstdlib>
#include <vector>

namespace{

std::vector<Pipeline::ScreenTriangle> MakeScene(){
    std::vector<Pipeline::ScreenTriangle> tris;
    std::srand(7);
    for(int i = 0; i < 24; i++){
        Pipeline::ScreenTriangle t{};
        for(int k = 0; k < 3; k++){
            t.v[k].x = std::rand() % 200;
            t.v[k].y = std::rand() % 150;
            t.v[k].z = -0.9f + 0.8f * (std::rand() % 100) / 100.0f;
            t.v[k].w = 1;
            t.v[k].u = static_cast<float>(std::rand() % 100) / 25.0f;
            t.v[k].v = static_cast<float>(std::rand() % 100) / 25.0f;
            t.v[k].nx = 0; t.v[k].ny = 0; t.v[k].nz = -1;
            t.v[k].wx = 0; t.v[k].wy = 0; t.v[k].wz = 0;
        }
        tris.push_back(t);
    }
    return tris;
}
}

TEST(TileRenderTest, TiledEqualsSerial){
    const auto scene = MakeScene();
    Texture tex(4, 4, std::vector<uint32_t>{
        0xFF0000FFu, 0xFF00FF00u, 0xFFFF0000u, 0xFF8040C0u,
        0xFF408020u, 0xFF203040u, 0xFFF0E0D0u, 0xFF102030u,
        0xFFFF00FFu, 0xFF00FFFFu, 0xFFFFFF00u, 0xFF123456u,
        0xFF654321u, 0xFFABCDEFu, 0xFF0F0F0Fu, 0xFFF0F0F0u}.data());

    FrameBuffer serial{200, 150};
    serial.clear();
    {
        Rasterizer rz{serial};
        for(const auto &t : scene){
            rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2], tex, nullptr);
        }
    }

    FrameBuffer tiledFb{200, 150};
    tiledFb.clear();
    SGE::Render::TileRenderer tiled{tiledFb};
    tiled.drawTextured(scene, tex, nullptr);

    for(std::size_t i = 0; i < 200u * 150u; i++){
        ASSERT_EQ(serial.colorData()[i], tiledFb.colorData()[i]) << "pixel " << i;
        ASSERT_EQ(serial.depthData()[i], tiledFb.depthData()[i]) << "depth " << i;
    }
}

TEST(TileRenderTest, ThreadCountExtremesStable){
    const auto scene = MakeScene();
    Texture tex(2, 2, std::vector<uint32_t>{
        0xFFFFFFFFu, 0xFF000000u, 0xFF000000u, 0xFFFFFFFFu}.data());

    FrameBuffer a{100, 100}, b{100, 100};
    a.clear(); b.clear();

    SGE::Render::TileRenderer ta{a}, tb{b};
    ta.drawTextured(scene, tex, nullptr, TextureFilter::Nearest, TextureWrap::Repeat, 0);
    tb.drawTextured(scene, tex, nullptr, TextureFilter::Nearest, TextureWrap::Repeat, 999);

    for(std::size_t i = 0; i < 100u * 100u; i++){
        EXPECT_EQ(a.colorData()[i], b.colorData()[i]);
    }
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
