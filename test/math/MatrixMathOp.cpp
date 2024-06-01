#include "Log.hpp"
#include "Matrix/Matrix.hpp"
#include "Matrix/MatrixUtils.hpp"
#include <gtest/gtest.h>

TEST(Matrix1DOp, Opeator){
    Matrix1D m1{1,1,1};
    Matrix1D m2{2,2,2};
    Matrix1D m3{3,3,3};

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

TEST(Matrix2DOp, Operator){
    Matrix2D m1{{1,1}, {1,1}};
    Matrix2D m2{{2,2}, {2,2}};
    Matrix2D m3{{3,3}, {3,3}};

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

TEST(Matrix3DOp, Operator){
    Matrix3D m1{{{1,1}, {1,1}}, {{1,1}, {1,1}}};
    Matrix3D m2{{{2,2}, {2,2}}, {{2,2}, {2,2}}};
    Matrix3D m3{{{3,3}, {3,3}}, {{3,3}, {3,3}}};

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
 
    EXPECT_EQ(m3.sum<int>(), 24);
}

TEST(Matrix4DOp, Operator){
    Matrix4D m1{{{{1,1}, {1,1}}, {{1,1}, {1,1}}},{{{1,1}, {1,1}}, {{1,1}, {1,1}}}};
    Matrix4D m2{{{{2,2}, {2,2}}, {{2,2}, {2,2}}},{{{2,2}, {2,2}}, {{2,2}, {2,2}}}};
    Matrix4D m3{{{{3,3}, {3,3}}, {{3,3}, {3,3}}},{{{3,3}, {3,3}}, {{3,3}, {3,3}}}};

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
 
    EXPECT_EQ(m3.sum<int>(), 48);
}

int main(int argc, char **argv){
    LOGI("Start matrix math operator ability test");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
