#include "Matrix.hpp"
#include "Log.hpp"
#include <gtest/gtest.h>

TEST(Matrix1DTest, Positive){
    LOGI("Testing Matrix1D base ability");
}
int main(int argc, char **argv){
    LOGI("Start matrix base ability test");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
