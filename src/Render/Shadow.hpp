#pragma once
#include "FrameBuffer.hpp"
#include "Transform.hpp"
#include <cmath>

namespace SGE::Render{

struct ShadowData{
    const FrameBuffer *depth{};
    Matrix4DBase<double> lightViewProj{};
    double bias{0.005};
    int pcfRadius{0};
};

inline Matrix4DBase<double> directionalLightVP(const Vector3DBase<double> &dir,
                                               const Vector3DBase<double> &center,
                                               double extent){
    const Vector3DBase<double> d = dir.normalize();
    const Vector3DBase<double> eye{center.x - d.x * extent * 2.0,
                                   center.y - d.y * extent * 2.0,
                                   center.z - d.z * extent * 2.0};
    const auto view = SGE::Math::lookAt(eye, center, Vector3DBase<double>{0, 1, 0});
    const auto proj = SGE::Math::orthographic(-extent, extent, -extent, extent,
                                              0.1, extent * 4.0);
    return proj.mul(view);
}

inline Matrix4DBase<double> pointLightVP(const Vector3DBase<double> &pos,
                                         const Vector3DBase<double> &target,
                                         double fovY, double aspect,
                                         double nearZ, double farZ){
    const auto view = SGE::Math::lookAt(pos, target, Vector3DBase<double>{0, 1, 0});
    const auto proj = SGE::Math::perspective(fovY, aspect, nearZ, farZ);
    return proj.mul(view);
}

struct CubeShadowData{
    const FrameBuffer *faces[6]{};
    Vector3DBase<double> lightPos{};
    double farPlane{50.0};
    double bias{0.005};
    int pcfRadius{0};

    static constexpr int kFaceCount = 6;
};

inline int cubeFaceIndex(const Vector3DBase<double> &dir){
    const double ax = std::abs(dir.x), ay = std::abs(dir.y), az = std::abs(dir.z);
    if(ax >= ay && ax >= az) return dir.x > 0 ? 0 : 1;
    if(ay >= az) return dir.y > 0 ? 2 : 3;
    return dir.z > 0 ? 4 : 5;
}

inline Matrix4DBase<double> cubeFaceVP(const Vector3DBase<double> &lightPos,
                                       int face){
    static const Vector3DBase<double> kDirs[6] = {
        {1, 0, 0}, {-1, 0, 0},
        {0, 1, 0}, {0, -1, 0},
        {0, 0, 1}, {0, 0, -1}};
    static const Vector3DBase<double> kUps[6] = {
        {0, -1, 0}, {0, -1, 0},
        {0, 0, 1}, {0, 0, 1},
        {0, -1, 0}, {0, -1, 0}};
    const Vector3DBase<double> target{lightPos.x + kDirs[face].x,
                                      lightPos.y + kDirs[face].y,
                                      lightPos.z + kDirs[face].z};
    const auto view = SGE::Math::lookAt(lightPos, target, kUps[face]);
    const auto proj = SGE::Math::perspective(M_PI / 2, 1.0, 0.1, 100.0);
    return proj.mul(view);
}

}
