#include "Camera.hpp"
#include <gtest/gtest.h>
#include <cmath>

namespace{
bool VecNear(const Vector3DBase<double> &v, double x, double y, double z, double eps = 1e-12){
    return std::fabs(v.x-x) < eps && std::fabs(v.y-y) < eps && std::fabs(v.z-z) < eps;
}
}

TEST(CameraTest, ForwardYawZeroFacesPlusZ){
    SGE::Render::Camera c{};
    c.yaw = 0; c.pitch = 0;
    EXPECT_TRUE(VecNear(c.forward(), 0, 0, 1));
}

TEST(CameraTest, ForwardPitchTiltsTowardY){
    SGE::Render::Camera c{};
    c.yaw = 0; c.pitch = 3.14159265358979 / 2;
    EXPECT_TRUE(VecNear(c.forward(), 0, 1, 0, 1e-9));
    c.pitch = -3.14159265358979 / 2;
    EXPECT_TRUE(VecNear(c.forward(), 0, -1, 0, 1e-9));
}

TEST(CameraTest, RightIsHorizontalPerpendicular){
    SGE::Render::Camera c{};
    c.yaw = 3.14159265358979 / 2; c.pitch = 0;
    EXPECT_TRUE(VecNear(c.right(), 0, 0, 1, 1e-9));

    c.yaw = 0.7; c.pitch = 0.3;
    const auto r = c.right();
    EXPECT_LT(std::fabs(r.y), 1e-12);
    const auto f = c.forward();
    EXPECT_NEAR(r.dot(f), 0.0, 1e-12);
}

TEST(CameraTest, ViewMatrixMatchesLookAt){
    SGE::Render::Camera c{};
    c.position = Vector3DBase<double>{0.5, -2, 3};
    c.yaw = 0.7; c.pitch = -0.4;
    const auto f = c.forward();
    auto expected = SGE::Math::lookAt(c.position,
        Vector3DBase<double>{c.position.x + f.x, c.position.y + f.y, c.position.z + f.z},
        Vector3DBase<double>{0, 1, 0});
    auto actual = c.viewMatrix();
    for(int r = 0; r < 4; r++){
        for(int cc = 0; cc < 4; cc++){
            EXPECT_NEAR(actual[0][0][r][cc], expected[0][0][r][cc], 1e-9) << r << "," << cc;
        }
    }
}

TEST(CameraTest, UpdateMovesAlongForward){
    SGE::Render::Camera c{};
    c.position = Vector3DBase<double>{0, 0, 0};
    c.yaw = 0; c.pitch = 0;
    SGE::Render::InputState in{};
    in.w = true;
    SGE::Render::update(c, in, 1.0);
    EXPECT_TRUE(VecNear(c.position, 0, 0, 3));
}

TEST(CameraTest, UpdateStrafesAlongRight){
    SGE::Render::Camera c{};
    c.position = Vector3DBase<double>{0, 0, 0};
    c.yaw = 0; c.pitch = 0;
    SGE::Render::InputState in{};
    in.d = true;
    SGE::Render::update(c, in, 1.0);
    EXPECT_TRUE(VecNear(c.position, -3, 0, 0));
}

TEST(CameraTest, UpdateTurnAndPitchClamp){
    SGE::Render::Camera c{};
    c.position = Vector3DBase<double>{0, 0, 0};
    SGE::Render::InputState in{};
    in.left = true;
    SGE::Render::update(c, in, 1.0);
    EXPECT_NEAR(c.yaw, -1.5, 1e-12);

    in.left = false; in.up = true;
    for(int i = 0; i < 10; i++){
        SGE::Render::update(c, in, 1.0);
    }
    EXPECT_NEAR(c.pitch, 3.14159265358979 / 2 - 0.01, 1e-12);
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
