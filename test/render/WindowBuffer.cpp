#include "Log.hpp"
#include "WindowBuffer.hpp"
#include "BufferManager.hpp"
#include "WindowDefine.hpp"
#include <gtest/gtest.h>

TEST(WindowBufferTest, BufferHasFullSize){
    const Size sz{4, 3};
    WindowBuffer<uint8_t> buf(sz, RenderFormat::RGBA8888);
    EXPECT_EQ(buf.size(0), 4 * 3 * 4);
    EXPECT_EQ(buf.buffer(), buf.buffer());
}

TEST(WindowBufferTest, ClearFillsAllPixels){
    const Size sz{4, 2};
    WindowBuffer<uint8_t> buf(sz, RenderFormat::RGBA8888);
    WindowColor c(1, 2, 3, 4);
    EXPECT_TRUE(buf.clear(c));
    for(auto i = 0; i < sz.height; i++){
        for(auto j = 0; j < sz.width; j++){
            const auto base = i * 16 + j * 4;
            EXPECT_EQ(buf.buffer()[base + 0], 4) << "i=" << i << " j=" << j;
            EXPECT_EQ(buf.buffer()[base + 1], 3) << "i=" << i << " j=" << j;
            EXPECT_EQ(buf.buffer()[base + 2], 2) << "i=" << i << " j=" << j;
            EXPECT_EQ(buf.buffer()[base + 3], 1) << "i=" << i << " j=" << j;
        }
    }
}

TEST(WindowBufferTest, FillReplacesContent){
    const Size sz{2, 1};
    WindowBuffer<uint8_t> buf(sz, RenderFormat::RGBA8888);
    uint8_t data[8] = {9,9,9,9,9,9,9,9};
    EXPECT_TRUE(buf.fill(data));
    for(auto i = 0; i < 8; i++){
        EXPECT_EQ(buf.buffer()[i], 9);
    }
}

TEST(WindowBufferTest, ClearBGRAOrder){
    const Size sz{2, 1};
    WindowBuffer<uint8_t> buf(sz, RenderFormat::BGRA8888);
    WindowColor c(1, 2, 3, 4);
    EXPECT_TRUE(buf.clear(c));
    for(auto j = 0; j < sz.width; j++){
        const auto base = j * 4;
        EXPECT_EQ(buf.buffer()[base + 0], 4);
        EXPECT_EQ(buf.buffer()[base + 1], 1);
        EXPECT_EQ(buf.buffer()[base + 2], 2);
        EXPECT_EQ(buf.buffer()[base + 3], 3);
    }
}

TEST(BufferManagerTest, SwapTogglesPrimary){
    const Size sz{2, 1};
    ASSERT_TRUE(!BufferManager::instance()->initialize(sz, RenderFormat::RGBA8888));
    BufferManager::instance()->clear(WindowColor(1, 2, 3, 4));
    auto before = BufferManager::instance()->getBuffer();
    BufferManager::instance()->swap();
    auto after = BufferManager::instance()->getBuffer();
    EXPECT_NE(before.get(), after.get());
}

int main(int argc, char **argv){
    LOGI("Start window buffer test");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}