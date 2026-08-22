#include "FrameBuffer.hpp"
#include <gtest/gtest.h>
#include <limits>

TEST(FrameBufferTest, Construction){
    FrameBuffer fb{8, 4};
    EXPECT_EQ(fb.width(), 8u);
    EXPECT_EQ(fb.height(), 4u);
    for(std::size_t i = 0; i < 8*4; i++){
        EXPECT_EQ(fb.colorData()[i], 0xFF000000u);
        EXPECT_FLOAT_EQ(fb.depthData()[i], std::numeric_limits<float>::max());
    }
}

TEST(FrameBufferTest, SetPixelRoundtrip){
    FrameBuffer fb{4, 4};
    fb.setPixel(2, 1, 0xFF0000FFu, 0.5f);
    EXPECT_EQ(fb.colorData()[1*4 + 2], 0xFF0000FFu);
    EXPECT_FLOAT_EQ(fb.depthData()[1*4 + 2], 0.5f);
}

TEST(FrameBufferTest, SetPixelOutOfBoundsIgnored){
    FrameBuffer fb{4, 4};
    fb.setPixel(4, 0, 0xFFFFFFFFu, 0.0f);
    fb.setPixel(0, 4, 0xFFFFFFFFu, 0.0f);
    fb.setPixel(100, 100, 0xFFFFFFFFu, 0.0f);
    SUCCEED();
}

TEST(FrameBufferTest, DepthTest){
    FrameBuffer fb{2, 2};
    fb.setPixel(0, 0, 0xFF00FF00u, 0.5f);
    fb.setPixel(0, 0, 0xFFFF0000u, 0.3f);
    EXPECT_EQ(fb.colorData()[0], 0xFFFF0000u);
    fb.setPixel(0, 0, 0xFF0000FFu, 0.7f);
    EXPECT_EQ(fb.colorData()[0], 0xFFFF0000u);
    fb.setPixel(0, 0, 0xFF0000FFu, 0.3f);
    EXPECT_EQ(fb.colorData()[0], 0xFFFF0000u);
}

TEST(FrameBufferTest, ClearResets){
    FrameBuffer fb{2, 2};
    fb.setPixel(0, 0, 0xFF123456u, 0.25f);
    fb.clear();
    for(std::size_t i = 0; i < 4; i++){
        EXPECT_EQ(fb.colorData()[i], 0xFF000000u);
        EXPECT_FLOAT_EQ(fb.depthData()[i], std::numeric_limits<float>::max());
    }
}

TEST(FrameBufferBlendTest, OpaqueMatchesSetPixel){
    FrameBuffer fb(2, 1);
    fb.setPixel(0, 0, 0xFF00FF00u, -1.0f);
    fb.blendPixel(1, 0, 0xFF00FF00u, -1.0f);
    EXPECT_EQ(fb.colorData()[0], fb.colorData()[1]);
}

TEST(FrameBufferBlendTest, SrcOverHalfAlpha){
    FrameBuffer fb(1, 1);
    // 偏差修正：简报原数据 setPixel(-1.0f)+blend(-0.5f) 中 src 比 dst 更远，
    // 按 setPixel 同款深度规则必被拒（与测试4语义冲突）；黑底改存 +0.5f 使 -0.5f 合法混合
    fb.setPixel(0, 0, 0xFF000000u, 0.5f);           // 黑底（较远）
    const uint32_t a = (128u << 24) | 0x808080u;    // α=128 灰128
    fb.blendPixel(0, 0, a, -0.5f);
    const uint32_t got = fb.colorData()[0];
    EXPECT_EQ((got >> 16) & 0xFF, ((128 * 128 + 0 * 127) + 127) / 255);
    EXPECT_EQ(got >> 24, 0xFFu);
}

TEST(FrameBufferBlendTest, AlphaZeroSkipsBoth){
    FrameBuffer fb(1, 1);
    fb.setPixel(0, 0, 0xFF0000FFu, -1.0f);
    // Fix R1(I-1)：-0.5 对 -1.0 更远会被深度守卫先行拒绝，α==0 分支零判别力；
    // 改 -2.0f（更近）确保真正走到 α==0 早退分支，深度断言锚定「双跳过」
    fb.blendPixel(0, 0, 0u, -2.0f);                 // α=0 且深度更近
    EXPECT_EQ(fb.colorData()[0], 0xFF0000FFu);
    EXPECT_FLOAT_EQ(fb.depthData()[0], -1.0f);
}

TEST(FrameBufferBlendTest, DepthStillGuardsMidAlpha){
    FrameBuffer fb(1, 1);
    fb.setPixel(0, 0, 0xFF0000FFu, -1.0f);
    const uint32_t a = (128u << 24) | 0x808080u;
    fb.blendPixel(0, 0, a, 0.5f);                   // 更远被拒
    EXPECT_EQ(fb.colorData()[0], 0xFF0000FFu);
}

TEST(FrameBufferBlendTest, OpaqueFarStillRejected){
    FrameBuffer fb(1, 1);
    fb.setPixel(0, 0, 0xFF0000FFu, -1.0f);
    fb.blendPixel(0, 0, 0xFF00FF00u, 0.5f);
    EXPECT_EQ(fb.colorData()[0], 0xFF0000FFu);
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
