#include "Log.hpp"
#include "Vector/Vector2DBase.hpp"
#include "Vector/Vector3DBase.hpp"
#include "Vector/Vector4DBase.hpp"
#include "Util/ToString.hpp"
#include <cassert>
#include <cstddef>
#include <gtest/gtest.h>

TEST(Vector2DBase, VectorOpt){
    Vector2DBase m1{1,1};
    Vector2DBase m2{2,2};
    Vector2DBase m3{3,3};

    LOGI("{}", std::to_string(m1));
    LOGI("{}", std::to_string(m2));
    LOGI("{}", std::to_string(m3));

    EXPECT_EQ(m1 + m2, m3);
    EXPECT_EQ(m1 + 1, m2);
    EXPECT_EQ(1 + m1, m2);

    EXPECT_EQ(m3 - m2, m1);
    EXPECT_EQ(m3 - 1, m2);
    EXPECT_EQ(3 - m1, m2);

    EXPECT_EQ(m1 * m2, m2);
    EXPECT_EQ(m1 * 1, m1);
    EXPECT_EQ(2 * m1, m2);

    EXPECT_EQ(m3 / m1, m3);
    EXPECT_EQ(m1 / 1, m1);
    EXPECT_EQ(3 / m3, m1);
 
    EXPECT_EQ(m3.sum<int>(), 6);
}

TEST(Vector3DBase, VectorOpt){
    Vector3DBase m1{1, 1, 1};
    Vector3DBase m2{2, 2, 2};
    Vector3DBase m3{3, 3, 3};

    LOGI("{}", std::to_string(m1));
    LOGI("{}", std::to_string(m2));
    LOGI("{}", std::to_string(m3));

    EXPECT_EQ(m1 + m2, m3);
    EXPECT_EQ(m1 + 1, m2);
    EXPECT_EQ(1 + m1, m2);

    EXPECT_EQ(m3 - m2, m1);
    EXPECT_EQ(m3 - 1, m2);
    EXPECT_EQ(3 - m1, m2);

    EXPECT_EQ(m1 * m2, m2);
    EXPECT_EQ(m1 * 1, m1);
    EXPECT_EQ(2 * m1, m2);

    EXPECT_EQ(m3 / m1, m3);
    EXPECT_EQ(m1 / 1, m1);
    EXPECT_EQ(3 / m3, m1);
 
    EXPECT_EQ(m3.sum<int>(), 9);
}

TEST(Vector4DBase, VectorOpt){
    Vector4DBase m1{1, 1, 1, 1};
    Vector4DBase m2{2, 2, 2, 2};
    Vector4DBase m3{3, 3, 3, 3};

    LOGI("{}", std::to_string(m1));
    LOGI("{}", std::to_string(m2));
    LOGI("{}", std::to_string(m3));

    EXPECT_EQ(m1 + m2, m3);
    EXPECT_EQ(m1 + 1, m2);
    EXPECT_EQ(1 + m1, m2);

    EXPECT_EQ(m3 - m2, m1);
    EXPECT_EQ(m3 - 1, m2);
    EXPECT_EQ(3 - m1, m2);

    EXPECT_EQ(m1 * m2, m2);
    EXPECT_EQ(m1 * 1, m1);
    EXPECT_EQ(2 * m1, m2);

    EXPECT_EQ(m3 / m1, m3);
    EXPECT_EQ(m1 / 1, m1);
    EXPECT_EQ(3 / m3, m1);
 
    EXPECT_EQ(m3.sum<int>(), 12);
}

int main(int argc, char **argv){
    LOGI("Start vector base ability test");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
