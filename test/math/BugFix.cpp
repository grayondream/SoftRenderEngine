#include "Log.hpp"
#include "Math.hpp"
#include "MathConst.hpp"
#include "FixPoint.hpp"
#include "PreComputeTable.hpp"
#include "Matrix.hpp"
#include "Vector.hpp"
#include "Primitive/Primitive.hpp"
#include "Color.hpp"
#include <gtest/gtest.h>

TEST(MathConstTest, FixedPointMag){
    EXPECT_EQ(Math::FP32Mag, 1 << Math::FP32Shift);
    EXPECT_EQ(Math::FP64Mag, int64_t{1} << Math::FP64Shift);
}

TEST(FixPointTest, ShiftAndMask){
    using namespace Math;
    EXPECT_EQ(FixedPointGetShift<FPNumber32>(), FP32Shift);
    EXPECT_EQ(FixedPointGetShift<FPNumber64>(), FP64Shift);
    EXPECT_EQ(FixedPointGetMag<FPNumber32>(), FP32Mag);
    EXPECT_EQ(FixedPointGetMag<FPNumber64>(), FP64Mag);
}

TEST(FixPointTest, ConvertRoundTrip){
    using namespace Math;
    const auto fp = NValue2FixedPoint<FPNumber32>(3.5);
    EXPECT_NEAR(FixedPoint2NValue(fp), 3.5, 1E-4);

    const auto dpart = FixedPointDp<FPNumber32>(fp);
    EXPECT_GT(dpart, 0);

    const auto wpart = FixedPointWp<FPNumber32>(fp);
    EXPECT_GT(wpart, 0);
}

TEST(PowTest, NegativeExponent){
    EXPECT_DOUBLE_EQ(Math::Pow(2.0, 0), 1.0);
    EXPECT_DOUBLE_EQ(Math::Pow(2.0, 3), 8.0);
    EXPECT_NEAR(Math::Pow(2.0, -1), 0.5, 1E-9);
    EXPECT_NEAR(Math::Pow(2.0, -3), 0.125, 1E-9);
}

TEST(VectorTest, Distance){
    Vector2DBase<double> v1{0, 0};
    Vector2DBase<double> v2{3, 4};
    EXPECT_DOUBLE_EQ(v1.distance(v2), 5.0);
}

TEST(VectorTest, DefaultConstructible){
    Vector2DBase<double> v2{};
    Vector3DBase<double> v3{};
    Vector4DBase<double> v4{};
    EXPECT_DOUBLE_EQ(v2.length(), 0.0);
    EXPECT_DOUBLE_EQ(v3.length(), 0.0);
    EXPECT_DOUBLE_EQ(v4.length(), 0.0);
}

TEST(VectorTest, ScalarAssignReturnsThis){
    Vector2DBase<double> v{};
    auto &ref = (v = 5.0);
    EXPECT_DOUBLE_EQ(ref.x, 5.0);
    EXPECT_DOUBLE_EQ(ref.y, 5.0);

    Vector3DBase<double> v3{};
    auto &ref3 = (v3 = 6.0);
    EXPECT_DOUBLE_EQ(ref3.x, 6.0);

    Vector4DBase<double> v4{};
    auto &ref4 = (v4 = 7.0);
    EXPECT_DOUBLE_EQ(ref4.x, 7.0);
}

TEST(MatrixCopyAssignTest, NoAliasing1D){
    Matrix1D a{1, 2, 3};
    Matrix1D b{9, 9, 9};
    a = b;
    a[0] = 100;
    EXPECT_EQ(b[0], 9);
}

TEST(MatrixCopyAssignTest, NoAliasing3D){
    Matrix3D a(std::vector(27, 1.0), 3, 3, 3);
    Matrix3D b(std::vector(27, 7.0), 3, 3, 3);
    a = b;
    a[0][0][0] = 100;
    EXPECT_EQ(b[0][0][0], 7);
}

TEST(MatrixCopyAssignTest, NoAliasing4D){
    Matrix4D a(std::vector(81, 1.0), 3, 3, 3, 3);
    Matrix4D b(std::vector(81, 9.0), 3, 3, 3, 3);
    a = b;
    a[0][0][0][0] = 100;
    EXPECT_EQ(b[0][0][0][0], 9);
}

TEST(StaticMatrixCompareTest, EqualNonSquare){
    StaticMatrix2DBase<double, 2, 3> m1{{1, 2, 3}, {4, 5, 6}};
    StaticMatrix2DBase<double, 2, 3> m2{{1, 2, 3}, {4, 5, 6}};
    EXPECT_TRUE(m1 == m2);

    StaticMatrix3DBase<double, 2, 2, 3> s1{{{1, 2, 3}, {4, 5, 6}}, {{7, 8, 9}, {10, 11, 12}}};
    StaticMatrix3DBase<double, 2, 2, 3> s2{{{1, 2, 3}, {4, 5, 6}}, {{7, 8, 9}, {10, 11, 12}}};
    EXPECT_TRUE(s1 == s2);
}

TEST(StaticMatrixCompareTest, NotEqual){
    StaticMatrix2DBase<double, 2, 3> m1{{1, 2, 3}, {4, 5, 6}};
    StaticMatrix2DBase<double, 2, 3> m2{{1, 2, 3}, {4, 5, 7}};
    EXPECT_FALSE(m1 == m2);
}

TEST(MatrixEyeTest, Dynamic2DZeroesOffDiagonal){
    Matrix2D m{{1, 2}, {3, 4}};
    m.eye();
    EXPECT_EQ(m[0][0], 1);
    EXPECT_EQ(m[0][1], 0);
    EXPECT_EQ(m[1][0], 0);
    EXPECT_EQ(m[1][1], 1);
}

TEST(MatrixEyeTest, Dynamic3DZeroesOffDiagonal){
    Matrix3D m{{{1,1},{1,1}},{{1,1},{1,1}},{{1,1},{1,1}}};
    m.eye();
    for(auto k = 0; k < 3; k++){
        for(auto i = 0; i < 2; i++){
            for(auto j = 0; j < 2; j++){
                EXPECT_EQ(m[k][i][j], (i == j) ? 1 : 0) << "k=" << k << " i=" << i << " j=" << j;
            }
        }
    }
}

TEST(MatrixEyeTest, Dynamic1DReturnsThis){
    Matrix1D m{1, 2, 3};
    auto &ref = m.eye();
    EXPECT_EQ(m[0], 1);
    EXPECT_EQ(m[1], 2);
    EXPECT_EQ(m[2], 3);
    EXPECT_EQ(&ref, &m);
}

TEST(MatrixEyeTest, Static1DReturnsThis){
    Matrix1x3 m{1, 2, 3};
    auto &ref = m.eye();
    EXPECT_EQ(m[0], 1);
    EXPECT_EQ(&ref, &m);
}

TEST(Matrix1DMulTest, Static2D){
    Matrix1D m1{2, 3};
    StaticMatrix2DBase<double, 2, 2> m2{{1, 2}, {3, 4}};
    auto ret = m1.mul(m2);
    EXPECT_EQ(ret.d1, 2);
    EXPECT_EQ(ret[0], 11);
    EXPECT_EQ(ret[1], 16);
}

TEST(ColorTest, ChannelsIndependent){
    Color32 c;
    c.r = 255;
    EXPECT_EQ(c.g, 0);
    EXPECT_EQ(c.b, 0);
    EXPECT_EQ(c.a, 0);
    c.g = 128;
    EXPECT_EQ(c.r, 255);
    EXPECT_EQ(c.g, 128);
    c.b = 64;
    EXPECT_EQ(c.b, 64);
    c.a = 32;
    EXPECT_EQ(c.a, 32);
    EXPECT_EQ(c.color[0], 255);
    EXPECT_EQ(c.color[1], 128);
    EXPECT_EQ(c.color[2], 64);
    EXPECT_EQ(c.color[3], 32);
}

int main(int argc, char **argv){
    LOGI("Start bug regression test");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}