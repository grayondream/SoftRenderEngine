#include "Texture.hpp"
#include <gtest/gtest.h>

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

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
