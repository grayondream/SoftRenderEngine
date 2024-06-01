#include "Log.hpp"
#include "Matrix/Matrix.hpp"
#include <gtest/gtest.h>

TEST(Matrix1DOp, Opeator){
    Matrix1D m1{2,2,2};
    Matrix1D m2{1,1,1};
    Matrix1D m3{3,3,3};
    EXPECT_EQ(m1 + m2, m3);
    EXPECT_EQ(m1 + 1, m3);
    EXPECT_EQ(1 + m1, m3);

    EXPECT_EQ(m3 - m2, m1);
    EXPECT_EQ(m1 - 1, m2);
    EXPECT_EQ(3 - m1, m2);

    EXPECT_EQ(m3 * m2, m3);
    EXPECT_EQ(m1 * 1, m1);
    EXPECT_EQ(2 * m2, m1);

    EXPECT_EQ(m3 / m2, m3);
    EXPECT_EQ(m1 / 1, m1);
    EXPECT_EQ(3 / m2, m3);

    EXPECT_EQ(m3.sum<int>(), 9);
}

int main(int argc, char **argv){
    LOGI("Start matrix math operator ability test");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
