#include "Matrix.hpp"
#include "Log.hpp"
#include "MatrixUtils.hpp"
#include <algorithm>
#include <gtest/gtest.h>

TEST(Matrix1DTest, CreateAndPrint){
    LOGI("Testing Matrix1D base ability");
    const int n1 = 5, n2 = 10, n3 = 20;
    Matrix1D m1(n1);
    Matrix1D m2(n2);
    Matrix1D m3(n3);
    std::fill_n(m1.getRawBuffer(), n1, 3.3);
    std::fill_n(m2.getRawBuffer(), n2, 2.2);
    std::fill_n(m3.getRawBuffer(), n3, 7.7);
    LOGI("m1 string: {}", std::to_string(m1));
    LOGI("m2 string: {}", std::to_string(m2));
    LOGI("m3 string: {}", std::to_string(m3));
}

int main(int argc, char **argv){
    LOGI("Start matrix base ability test");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
