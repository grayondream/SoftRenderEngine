#include "Log.hpp"
#include "MathConst.hpp"
#include "Matrix/Matrix.hpp"
#include "Util/ToString.hpp"
#include <cmath>
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

TEST(Matrix1DOp, ComplexOperator){
    Matrix1D m1{2,3};
    Matrix2D m2{{1,2}, {3,4}};
    Matrix1D m3{11, 16};
    Matrix2D m4{{2}, {3}};
    EXPECT_EQ(m1.mul(m2), m3);
    EXPECT_EQ(m1.transpose(), m4);
}


TEST(Matrix2DOp, ComplexOperator){
    {
        Matrix2D m1{{1,2},{3,4}};
        Matrix2D m2{{1,2, 3}, {4, 5, 6}};
        Matrix2D m3{{9, 12, 15},{19,26,33}};
        Matrix2D m4{{1,3}, {2,4}};
        EXPECT_EQ(m1.mul(m2), m3);
        EXPECT_EQ(m1.transpose(), m4);
    }

    {
        Matrix2D m1{{1,2},{3,4}};
        Matrix2D m2{{3,4}, {1,2}};
        Matrix2D m3{{1,2}, {1,2}};
        EXPECT_EQ(m1.swapRows(0, 1), m2);
        EXPECT_EQ(m1.runk(), 2);
        EXPECT_EQ(m1.eye().runk(), 2);
        EXPECT_EQ(m3.runk(), 1);
        EXPECT_EQ(m3.fill(0).runk(), 0);
    }

    {
        Matrix2D m1{{1,2},{3,4}};
        Matrix2D m2{{-2,1},{3.0/2,-1.0/2}};
        if(m1.invert()){
            EXPECT_EQ(m1.equal(m2, [](const double v1, const double v2){ return std::fabs(v1 - v2) < Math::EpsilonE7;}), true);
        }
        
    }
}

int main(int argc, char **argv){
    LOGI("Start matrix math operator ability test");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
