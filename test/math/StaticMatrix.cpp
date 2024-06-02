#include "Log.hpp"
#include "Matrix.hpp"
#include "ToString.hpp"
#include <gtest/gtest.h>

TEST(Matrix1x4Op, Opeator){
    Matrix1x3 m1{1,1,1};
    Matrix1x3 m2{2,2,2};
    Matrix1x3 m3{3,3,3};

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

TEST(Matrix2x2Op, Operator){
    Matrix2x2 m1{{1,1}, {1,1}};
    Matrix2x2 m2{{2,2}, {2,2}};
    Matrix2x2 m3{{3,3}, {3,3}};

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

TEST(Matrix2x2x2Op, Operator){
    Matrix2x2x2 m1{{{1,1}, {1,1}}, {{1,1}, {1,1}}};
    Matrix2x2x2 m2{{{2,2}, {2,2}}, {{2,2}, {2,2}}};
    Matrix2x2x2 m3{{{3,3}, {3,3}}, {{3,3}, {3,3}}};

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

TEST(Matrix2x2x2x2Op, Operator){
    Matrix2x2x2x2 m1{{{{1,1}, {1,1}}, {{1,1}, {1,1}}},{{{1,1}, {1,1}}, {{1,1}, {1,1}}}};
    Matrix2x2x2x2 m2{{{{2,2}, {2,2}}, {{2,2}, {2,2}}},{{{2,2}, {2,2}}, {{2,2}, {2,2}}}};
    Matrix2x2x2x2 m3{{{{3,3}, {3,3}}, {{3,3}, {3,3}}},{{{3,3}, {3,3}}, {{3,3}, {3,3}}}};

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
    LOGI("Start static matrix math operator ability test");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
