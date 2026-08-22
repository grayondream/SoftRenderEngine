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

TEST(TransformTest, LookAt){
    Matrix4DBase<double> m = SGE::Math::lookAt(
        Vector3DBase<double>{0.0, 0.0, 0.0},
        Vector3DBase<double>{0.0, 0.0, -1.0},
        Vector3DBase<double>{0.0, 1.0, 0.0});
    // Eye position maps to origin
    Vector4DBase<double> v{0.0, 0.0, 0.0, 1.0};
    auto result = m.mul(v);
    EXPECT_NEAR(result.x, 0.0, 1e-10);
    EXPECT_NEAR(result.y, 0.0, 1e-10);
    EXPECT_NEAR(result.z, 0.0, 1e-10);
    EXPECT_NEAR(result.w, 1.0, 1e-10);
}

TEST(TransformTest, LookAtForward){
    // Camera at origin looking down -Z (OpenGL convention):
    // a point in front of the camera keeps negative view-space Z
    Matrix4DBase<double> m = SGE::Math::lookAt(
        Vector3DBase<double>{0.0, 0.0, 0.0},
        Vector3DBase<double>{0.0, 0.0, -1.0},
        Vector3DBase<double>{0.0, 1.0, 0.0});
    Vector4DBase<double> v{0.0, 0.0, -5.0, 1.0};
    auto result = m.mul(v);
    EXPECT_NEAR(result.z, -5.0, 1e-10);
    EXPECT_NEAR(result.x, 0.0, 1e-10);
    EXPECT_NEAR(result.y, 0.0, 1e-10);
}

TEST(TransformTest, Perspective){
    Matrix4DBase<double> m = SGE::Math::perspective(M_PI / 4.0, 800.0/600.0, 0.1, 100.0);
    // Near plane center maps to z=-1 in NDC
    Vector4DBase<double> vNear{0.0, 0.0, -0.1, 1.0};
    auto rNear = m.mul(vNear);
    EXPECT_NEAR(rNear.z / rNear.w, -1.0, 1e-10);

    // Far plane center maps to z=+1 in NDC
    Vector4DBase<double> vFar{0.0, 0.0, -100.0, 1.0};
    auto rFar = m.mul(vFar);
    EXPECT_NEAR(rFar.z / rFar.w, 1.0, 1e-10);
}

TEST(TransformTest, Orthographic){
    Matrix4DBase<double> m = SGE::Math::orthographic(-1.0, 1.0, -1.0, 1.0, 0.1, 100.0);
    Vector4DBase<double> v{0.5, -0.5, -50.0, 1.0};
    auto result = m.mul(v);
    EXPECT_NEAR(result.x, 0.5, 1e-10);
    EXPECT_NEAR(result.y, -0.5, 1e-10);
    // NDC z = (-2z - (f+n)) / (f-n); near->-1, far->+1
    EXPECT_NEAR(result.z, (-2.0 * (-50.0) - (100.0 + 0.1)) / (100.0 - 0.1), 1e-6);
    EXPECT_NEAR(result.w, 1.0, 1e-10);
}

TEST(TransformTest, MVPPipeline){
    Matrix4DBase<double> model = SGE::Math::translation(0.0, 0.0, -5.0);
    Matrix4DBase<double> view = SGE::Math::lookAt(
        Vector3DBase<double>{0.0, 0.0, 0.0},
        Vector3DBase<double>{0.0, 0.0, -1.0},
        Vector3DBase<double>{0.0, 1.0, 0.0});
    Matrix4DBase<double> proj = SGE::Math::perspective(M_PI / 4.0, 1.0, 0.1, 100.0);
    auto mvp = proj.mul(view).mul(model);
    // Point at world origin lands at (0,0,-5); projects to screen center
    Vector4DBase<double> v{0.0, 0.0, 0.0, 1.0};
    auto result = mvp.mul(v);
    EXPECT_NEAR(result.x / result.w, 0.0, 1e-6);
    EXPECT_NEAR(result.y / result.w, 0.0, 1e-6);
    // z/w should be inside [-1, 1]
    EXPECT_TRUE(result.z / result.w > -1.0 && result.z / result.w < 1.0);
}

TEST(TransformNormalMatrixTest, PureRotationStaysSame){
    auto model = SGE::Math::translation(0.0, 0.0, 0.0)
        .mul(SGE::Math::rotationY(0.7))
        .mul(SGE::Math::rotationX(0.4));
    auto nm = SGE::Math::normalMatrix(model);
    for(int r = 0; r < 3; r++){
        for(int c = 0; c < 3; c++){
            EXPECT_NEAR(nm[0][r][c], model[0][0][r][c], 1e-9) << r << "," << c;
        }
    }
}

TEST(TransformNormalMatrixTest, NonUniformScale){
    auto model = SGE::Math::translation(1.0, 2.0, 3.0)
        .mul(SGE::Math::scale(2.0, 1.0, 1.0));
    auto nm = SGE::Math::normalMatrix(model);
    EXPECT_NEAR(nm[0][0][0], 0.5, 1e-9);
    EXPECT_NEAR(nm[0][1][1], 1.0, 1e-9);
    EXPECT_NEAR(nm[0][2][2], 1.0, 1e-9);
    EXPECT_NEAR(nm[0][0][1], 0.0, 1e-9);
    EXPECT_NEAR(nm[0][1][0], 0.0, 1e-9);
}

TEST(TransformNormalMatrixTest, SingularFallsBackToIdentity){
    Matrix4DBase<double> zero(1,1,4,4);
    zero.fill(0.0);
    auto nm = SGE::Math::normalMatrix(zero);
    EXPECT_NEAR(nm[0][0][0], 1.0, 1e-9);
    EXPECT_NEAR(nm[0][1][1], 1.0, 1e-9);
    EXPECT_NEAR(nm[0][2][2], 1.0, 1e-9);
    EXPECT_NEAR(nm[0][0][1], 0.0, 1e-9);
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
