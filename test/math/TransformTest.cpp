#include "Transform.hpp"
#include <gtest/gtest.h>
#include <cmath>

TEST(TransformTest, Translation){
    Matrix4DBase<double> m = SGE::Math::translation(1.0, 2.0, 3.0);
    Vector4DBase<double> v{0.0, 0.0, 0.0, 1.0};
    auto result = m.mul(v);
    EXPECT_DOUBLE_EQ(result.x, 1.0);
    EXPECT_DOUBLE_EQ(result.y, 2.0);
    EXPECT_DOUBLE_EQ(result.z, 3.0);
    EXPECT_DOUBLE_EQ(result.w, 1.0);
}

TEST(TransformTest, RotationX){
    Matrix4DBase<double> m = SGE::Math::rotationX(M_PI / 2.0);
    Vector4DBase<double> v{0.0, 1.0, 0.0, 1.0};
    auto result = m.mul(v);
    EXPECT_NEAR(result.x, 0.0, 1e-10);
    EXPECT_NEAR(result.y, 0.0, 1e-10);
    EXPECT_NEAR(result.z, 1.0, 1e-10);
    EXPECT_NEAR(result.w, 1.0, 1e-10);
}

TEST(TransformTest, RotationY){
    Matrix4DBase<double> m = SGE::Math::rotationY(M_PI / 2.0);
    Vector4DBase<double> v{1.0, 0.0, 0.0, 1.0};
    auto result = m.mul(v);
    EXPECT_NEAR(result.x, 0.0, 1e-10);
    EXPECT_NEAR(result.y, 0.0, 1e-10);
    EXPECT_NEAR(result.z, -1.0, 1e-10);
    EXPECT_NEAR(result.w, 1.0, 1e-10);
}

TEST(TransformTest, RotationZ){
    Matrix4DBase<double> m = SGE::Math::rotationZ(M_PI / 2.0);
    Vector4DBase<double> v{1.0, 0.0, 0.0, 1.0};
    auto result = m.mul(v);
    EXPECT_NEAR(result.x, 0.0, 1e-10);
    EXPECT_NEAR(result.y, 1.0, 1e-10);
    EXPECT_NEAR(result.z, 0.0, 1e-10);
    EXPECT_NEAR(result.w, 1.0, 1e-10);
}

TEST(TransformTest, Scale){
    Matrix4DBase<double> m = SGE::Math::scale(2.0, 3.0, 4.0);
    Vector4DBase<double> v{1.0, 1.0, 1.0, 1.0};
    auto result = m.mul(v);
    EXPECT_DOUBLE_EQ(result.x, 2.0);
    EXPECT_DOUBLE_EQ(result.y, 3.0);
    EXPECT_DOUBLE_EQ(result.z, 4.0);
    EXPECT_DOUBLE_EQ(result.w, 1.0);
}

TEST(TransformTest, MatrixMul){
    Matrix4DBase<double> t = SGE::Math::translation(1.0, 0.0, 0.0);
    Matrix4DBase<double> s = SGE::Math::scale(2.0, 2.0, 2.0);
    auto ts = t.mul(s);
    Vector4DBase<double> v{1.0, 1.0, 1.0, 1.0};
    auto result = ts.mul(v);
    EXPECT_NEAR(result.x, 3.0, 1e-10);
    EXPECT_NEAR(result.y, 2.0, 1e-10);
    EXPECT_NEAR(result.z, 2.0, 1e-10);
    EXPECT_NEAR(result.w, 1.0, 1e-10);
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
