#include "Matrix/Matrix.hpp"
#include "Log.hpp"
#include "Matrix/Matrix.hpp"
#include "Util/ToString.hpp"
#include <cassert>
#include <cstddef>
#include <gtest/gtest.h>


static void LogMatrix1D(const Matrix1D &mat, const std::string tag = {}){
    LOGI("Matrix {} total size: {}, d1 = {}\n{}", tag, mat.size(), mat.d1, std::to_string(mat));
}

static void LogMatrix2D(const Matrix2D &mat, const std::string tag = {}){
    LOGI("Matrix {} total size: {}, d1 = {}, d2 = {}\n{}", tag, mat.size(), mat.d1, mat.d2, std::to_string(mat));
}

static void LogMatrix3D(const Matrix3D &mat, const std::string tag = {}){
    LOGI("Matrix {} total size: {}, d1 = {}, d2 = {}, d3 = {}\n{}", tag, mat.size(), mat.d1, mat.d2, mat.d3, std::to_string(mat));
}

static void LogMatrix4D(const Matrix4D &mat, const std::string tag = {}){
    LOGI("Matrix {} total size: {}, d1 = {}, d2 = {}, d3 = {}, d4 = {}\n{}", tag, mat.size(), mat.d1, mat.d2, mat.d3, mat.d4, std::to_string(mat));
}

template<class T>
static void LogMatrix(const T &mat, const std::string tag = {}){
    if constexpr(std::is_same_v<T, Matrix1D>){
        return LogMatrix1D(mat, tag);
    }else if constexpr(std::is_same_v<T, Matrix2D>){
        return LogMatrix2D(mat, tag);
    }else if constexpr(std::is_same_v<T, Matrix3D>){
        return LogMatrix3D(mat, tag);
    }else if constexpr(std::is_same_v<T, Matrix4D>){
        return LogMatrix4D(mat, tag);
    }

    assert(0);
}

TEST(Matrix1DTest, CreateAndPrint){
    LOGI("Testing Matrix1D base ability");
    const int n1 = 5, n2 = 7, n3 = 10;
    Matrix1D m1(std::vector(n1, 2.2));
    Matrix1D m2(std::vector(n2, 3.3));
    Matrix1D m3(std::vector(n3, 4.2));
    
    Matrix1D m4({1,2,3,4,5,6,7,8,9,10,11}) ;
    LogMatrix(m1);
    LogMatrix(m2);
    LogMatrix(m3);
    LogMatrix(m4);
    EXPECT_EQ(m1[0], 2.2);
    EXPECT_EQ(m2[2], 3.3);
    EXPECT_EQ(m3[3], 4.2);
    EXPECT_EQ(m4[2], 3);
}

TEST(Matrix2DTest, CreateAndPrint){
    LOGI("Testing Matrix2D base ability");
    const std::size_t n1 = 25, n2 = 49, n3 = 100;
    std::vector<double> vec1(n1, 10), vec2(n2, 11), vec3(n3, 22);
    Matrix2D m1(vec1, 5, 5);
    Matrix2D m2(vec2, 7, 7);
    Matrix2D m3(vec3, 10, 10);
    LogMatrix(m1);
    LogMatrix(m2);
    LogMatrix(m3);

    EXPECT_EQ(m1[1][1], 10);
    EXPECT_EQ(m2[2][2], 11);
    EXPECT_EQ(m3[3][3], 22);
};

TEST(Matrix3DTest, CreateAndPrint){
    LOGI("Testing Matrix3D base ability");
    const std::size_t n1 = 27, n2 = 8 * 8 * 8;
    std::vector<double> vec1(n1, 10), vec2(n2, 11);
    Matrix3D m1(vec1, 3, 3, 3);
    Matrix3D m2(vec2, 8, 8, 8);
    LogMatrix(m1);
    LogMatrix(m2);

    EXPECT_EQ(m1[0][0][0], 10);
    EXPECT_EQ(m2[0][0][0], 11);
}

TEST(Matrix4DTest, CreateAndPrint){
    LOGI("Testing Matrix4D base ability");
    const std::size_t n1 = 81, n2 = 8 * 8 * 8 * 8;
    std::vector<double> vec1(n1, 10), vec2(n2, 11);
    Matrix4D m1(vec1, 3, 3, 3, 3);
    Matrix4D m2(vec2, 8, 8, 8, 8);
    LogMatrix(m1);
    LogMatrix(m2);
    EXPECT_EQ(m1[0][0][0][0], 10);
    EXPECT_EQ(m2[0][0][0][0], 11);
}

TEST(Matrix3DTest, IndexAt){
    LOGI("Testing Matrix4D index at");
    const std::size_t n1 = 27;
    std::vector<double> vec1(n1, 5);
    Matrix3D m1(vec1, 3, 3, 3);
    LogMatrix(m1);
    m1[0][0][0] = 1;
    m1[1][1][1] = 2;
    m1[2][2][2] = 3;
    LogMatrix(m1);

    EXPECT_EQ(m1[0][0][0], 1);
    EXPECT_EQ(m1[1][1][1], 2);
    EXPECT_EQ(m1[2][2][2], 3);
}

TEST(StaticMatrixTest, CreateAndPrint){
    LOGI("Testing Static Matrix 4D create and print");
    Matrix1x4 mat({1, 2, 3, 4});
    LOGI("Matrix {} total size: {}, d1 = {}\n{}", "", mat.size(), mat.d1, std::to_string(mat));

    Matrix3x3 m2({1, 2, 3, 4,5,6,7,8,9});
    LOGI("Matrix {} total size: {}, d1 = {}, d2 = {}\n{}", "", mat.size(), m2.d1, m2.d2, std::to_string(m2));

    StaticMatrix3DBase<double, 2, 2, 2> m3({1, 2, 3, 4,5,6,7,8});
    m3[0][0][0] = -1;
    m3[0][1][1] = -2;
    m3[1][0][0] = -3;
    LOGI("Matrix {} total size: {}, d1 = {}, d2 = {}\n{}", "", mat.size(), m3.d1, m3.d2, std::to_string(m3));
    EXPECT_EQ(m3[0][0][0], -1);
    EXPECT_EQ(m3[0][1][1], -2);
    EXPECT_EQ(m3[1][0][0], -3);
}

TEST(StaticMatrixTest, InitializerCreate){
    LOGI("Testing Static Matrix create and print");
    Matrix2x2 m1 = {{1,2},{3,4}};
    LOGI("Matrix2x2 content:\n{}", std::to_string(m1));

    EXPECT_EQ(m1[0][0], 1);
    EXPECT_EQ(m1[0][1], 2);

    Matrix2x2x2 m2 = {{{2,3},{2,2}}, {{2,2},{2,2}}};
    LOGI("Matrix2x2x2 content:\n{}", std::to_string(m2));

    EXPECT_EQ(m2[0][0][0], 2);
    EXPECT_EQ(m2[0][0][1], 3);
}

TEST(DynamicMatrixTest, InitializerCreate){
    LOGI("Testing Static Matrix create and print");
    Matrix2DBase<double> m1{{11,22},{33,44}};
    LOGI("Matrix2x2 content:\n{}", std::to_string(m1));
    EXPECT_EQ(m1[0][0], 11);
    EXPECT_EQ(m1[0][1], 22);

    Matrix3DBase<double> m2 = {{{22,2},{2,11}}, {{2,2},{2,2}}};
    LOGI("Matrix2x2x2 content:\n{}", std::to_string(m2));
    EXPECT_EQ(m2[0][0][0], 22);
    EXPECT_EQ(m2[0][1][1], 11);
}

TEST(MatrixDefaultConstructionTest, EmptyBracesDoesNotCrash){
    Matrix1DBase<double> a{};
    EXPECT_TRUE(a.empty());
    Matrix2DBase<double> b{};
    EXPECT_TRUE(b.empty());
    Matrix3DBase<double> c{};
    EXPECT_TRUE(c.empty());
    Matrix4DBase<double> d{};
    EXPECT_TRUE(d.empty());
}

int main(int argc, char **argv){
    LOGI("Start matrix base ability test");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
