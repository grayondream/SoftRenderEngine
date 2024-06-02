#include "Log.hpp"
#include "Math.hpp"
#include "Geometry.hpp"
#include "MathConst.hpp"
#include "PreComputeTable.hpp"
#include <cmath>
#include <gtest/gtest.h>

TEST(Math, Cos){
    for(auto i = 0;i <  Math::PrecomputedCosineTable.size();i ++){
        auto e =  Math::PrecomputedCosineTable[i];
        LOGI("degree {}'s cosine value is  {}", i, e);
        EXPECT_LT(std::abs(e - std::cos(Math::Degree2Radius(i))), Math::EpsilonE7);
    }

    for(auto i = 0;i <  Math::PrecomputedSinTable.size();i ++){
        auto e =  Math::PrecomputedSinTable[i];
        LOGI("degree {}'s sin value is  {}", i, e);
        EXPECT_LT(std::abs(e - std::sin(Math::Degree2Radius(i))), Math::EpsilonE7);
    }

    for(auto i = 0;i <  Math::PrecomputedTanTable.size();i ++){
        auto e =  Math::PrecomputedTanTable[i];
        LOGI("degree {}'s tan value is  {}", i, e);
        if(i != 90 && i != 270){
            EXPECT_LT(std::abs(e - std::tan(Math::Degree2Radius(i))), Math::EpsilonE7);
        }else{
            EXPECT_GT(std::abs(e), 2E10);
        }
    }
}

int main(int argc, char **argv){
    LOGI("Start vector base ability test");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}