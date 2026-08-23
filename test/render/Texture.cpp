#include "Texture.hpp"
#include <gtest/gtest.h>
#include <vector>

namespace{
constexpr uint32_t kBlack = 0xFF000000u;
constexpr uint32_t kRed   = 0xFFFF0000u;
constexpr uint32_t kGreen = 0xFF00FF00u;
constexpr uint32_t kBlue  = 0xFF0000FFu;
constexpr uint32_t kWhite = 0xFFFFFFFFu;
constexpr uint32_t kGray(uint32_t v){
    return 0xFF000000u | (v << 16) | (v << 8) | v;
}
}

TEST(TextureTest, EmptyReturnsBlack){
    Texture t{};
    EXPECT_EQ(t.width(), 0u);
    EXPECT_EQ(t.height(), 0u);
    EXPECT_EQ(t.sample(0.5, 0.5), kBlack);
    EXPECT_EQ(t.sample(0.5, 0.5, TextureFilter::Nearest), kBlack);
    EXPECT_EQ(t.sample(0.5, 0.5, TextureFilter::Bilinear), kBlack);
}

TEST(TextureTest, MemoryConstructorStoresPixels){
    const uint32_t px[4] = {kRed, kGreen, kBlue, kWhite};
    Texture t(2, 2, px);
    ASSERT_EQ(t.width(), 2u);
    ASSERT_EQ(t.height(), 2u);

    EXPECT_EQ(t.sample(0.24, 0.24, TextureFilter::Nearest), kRed);
    EXPECT_EQ(t.sample(0.75, 0.26, TextureFilter::Nearest), kGreen);
    EXPECT_EQ(t.sample(0.25, 0.76, TextureFilter::Nearest), kBlue);
    EXPECT_EQ(t.sample(0.75, 0.75, TextureFilter::Nearest), kWhite);
}

TEST(TextureTest, WrapRepeat){
    const uint32_t px[4] = {kRed, kGreen, kBlue, kWhite};
    Texture t(2, 2, px);

    EXPECT_EQ(t.sample(-0.25, 0.26, TextureFilter::Nearest, TextureWrap::Repeat), kGreen);
    EXPECT_EQ(t.sample( 1.25, 0.26, TextureFilter::Nearest, TextureWrap::Repeat), kRed);
    EXPECT_EQ(t.sample( 0.26,-0.25, TextureFilter::Nearest, TextureWrap::Repeat), kBlue);
}

TEST(TextureTest, WrapClamp){
    const uint32_t px[4] = {kRed, kGreen, kBlue, kWhite};
    Texture t(2, 2, px);

    EXPECT_EQ(t.sample(-0.25, 0.26, TextureFilter::Nearest, TextureWrap::Clamp), kRed);
    EXPECT_EQ(t.sample( 1.25, 0.26, TextureFilter::Nearest, TextureWrap::Clamp), kGreen);
    EXPECT_EQ(t.sample( 0.26, 1.25, TextureFilter::Nearest, TextureWrap::Clamp), kBlue);
    EXPECT_EQ(t.sample( 1.25, 1.25, TextureFilter::Nearest, TextureWrap::Clamp), kWhite);
}

TEST(TextureTest, BilinearRepeatAcrossSeam){
    const uint32_t px[4] = {kRed, kGreen, kBlue, kWhite};
    Texture t(2, 2, px);

    EXPECT_EQ(t.sample(-0.125, 0.25, TextureFilter::Bilinear, TextureWrap::Repeat), 0xFF40BF00u);
    EXPECT_EQ(t.sample( 1.125, 0.25, TextureFilter::Bilinear, TextureWrap::Repeat), 0xFFBF4000u);
}

TEST(TextureTest, BilinearExactAtTexelCenter){
    const uint32_t px[4] = {kRed, kGreen, kBlue, kWhite};
    Texture t(2, 2, px);

    EXPECT_EQ(t.sample(0.25, 0.25, TextureFilter::Bilinear), kRed);
    EXPECT_EQ(t.sample(0.75, 0.25, TextureFilter::Bilinear), kGreen);
}

TEST(TextureTest, BilinearBlendGrayLevels){
    const uint32_t px[4] = {kGray(100), kGray(200), kGray(0), kGray(200)};
    Texture t(2, 2, px);

    EXPECT_EQ(t.sample(0.375, 0.25, TextureFilter::Bilinear), kGray(125));
    EXPECT_EQ(t.sample(0.5,   0.5,  TextureFilter::Bilinear), kGray(125));
}

TEST(TextureTest, LoadFromFile){
    Texture t = Texture::loadFromFile("test/assets/test_4x4.png");
    ASSERT_EQ(t.width(), 4u);
    ASSERT_EQ(t.height(), 4u);

    for(int y = 0; y < 4; y++){
        for(int x = 0; x < 4; x++){
            const uint32_t expect = 0xFF000000u
                                  | (static_cast<uint32_t>(x*60) << 16)
                                  | (static_cast<uint32_t>(y*60) << 8)
                                  | 128u;
            const uint32_t got = t.sample((x + 0.5) / 4.0, (y + 0.5) / 4.0,
                                          TextureFilter::Nearest, TextureWrap::Clamp);
            EXPECT_EQ(got, expect) << "x=" << x << " y=" << y;
        }
    }
}

TEST(TextureTest, LoadFromFileMissingGivesEmpty){
    Texture t = Texture::loadFromFile("no_such_file.png");
    EXPECT_EQ(t.width(), 0u);
    EXPECT_EQ(t.sample(0.5, 0.5), kBlack);
}

TEST(TextureDeathTest, NullptrNonZeroAsserts){
    EXPECT_DEATH(Texture t(4, 4, nullptr), "");
}

TEST(TextureTest, HugeUvDoesNotOverflowAndReturnsTexel){
    Texture t(2, 2, std::vector<uint32_t>{
        0xFF0000FFu, 0xFF00FF00u, 0xFFFF0000u, 0xFFFFFFFFu}.data());
    const double huge = 1e300;
    const uint32_t a = t.sample(huge, huge, TextureFilter::Nearest, TextureWrap::Repeat);
    EXPECT_EQ((a >> 24) & 0xFF, 0xFF);
    const uint32_t b = t.sample(huge, huge, TextureFilter::Bilinear, TextureWrap::Repeat);
    EXPECT_EQ((b >> 24) & 0xFF, 0xFF);
}

TEST(TextureMipTest, ChainLevelsAndAverage){
    std::vector<uint32_t> px(64);
    for(std::size_t i = 0; i < 64; i++){
        px[i] = (i % 2 == 0) ? 0xFFFF0000u : 0xFF0000FFu;
    }
    Texture t(8, 8, px.data());
    t.buildMipChain();
    EXPECT_EQ(t.mipCount(), 4);

    const uint32_t c = t.sampleTrilinear(0.5, 0.5, 2.0f, TextureWrap::Repeat);
    EXPECT_NEAR(static_cast<int>((c >> 16) & 0xFF), 255, 130);
}

TEST(TextureMipTest, TrilinearBlendsBetweenLevels){
    Texture t(2, 2, std::vector<uint32_t>{
        0xFFFFFFFFu, 0xFFFFFFFFu,
        0xFFFFFFFFu, 0xFFFFFFFFu}.data());
    t.buildMipChain();
    EXPECT_EQ(t.mipCount(), 2);

    const uint32_t l0 = t.sampleTrilinear(0.25, 0.25, 0.0f, TextureWrap::Clamp);
    EXPECT_EQ((l0 >> 16) & 0xFF, 255);

    const uint32_t half = t.sampleTrilinear(0.25, 0.25, 0.5f, TextureWrap::Clamp);
    EXPECT_EQ((half >> 16) & 0xFF, 255);
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
