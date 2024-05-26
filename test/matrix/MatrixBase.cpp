#include "Matrix.hpp"
#include "Log.hpp"
#include "MatrixUtils.hpp"
#include <algorithm>
#include <cstddef>
#include <gtest/gtest.h>
#include <type_traits>

static void LogMatrix1D(const Matrix1D &mat, const std::string tag = {}){
    LOGI("Matrix {} total size: {}, d1 = {}\n{}", tag, mat.size(), mat.m_d1, std::to_string(mat));
}

static void LogMatrix2D(const Matrix2D &mat, const std::string tag = {}){
    LOGI("Matrix {} total size: {}, d1 = {}, d2 = {}\n{}", tag, mat.size(), mat.m_d1, mat.m_d2, std::to_string(mat));
}

template<class T>
static void LogMatrix(const T &mat, const std::string tag = {}){
    if constexpr(std::is_same_v<T, Matrix1D>){
        return LogMatrix1D(mat, tag);
    }else if constexpr(std::is_same_v<T, Matrix2D>){
        return LogMatrix2D(mat, tag);
    }
}

TEST(Matrix1DTest, CreateAndPrint){
    LOGI("Testing Matrix1D base ability");
    const int n1 = 5, n2 = 7, n3 = 10;
    Matrix1D m1(n1);
    Matrix1D m2(n2);
    Matrix1D m3(n3);
    std::fill_n(m1.getRawBuffer(), n1, 3.3);
    std::fill_n(m2.getRawBuffer(), n2, 2.2);
    std::fill_n(m3.getRawBuffer(), n3, 7.7);
    Matrix1D m4({1,2,3,4,5,6,7,8,9,10,11}) ;
    LogMatrix(m1);
    LogMatrix(m2);
    LogMatrix(m3);
    LogMatrix(m4);
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
};

int main(int argc, char **argv){
    LOGI("Start matrix base ability test");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
