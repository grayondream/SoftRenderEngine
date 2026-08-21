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

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
