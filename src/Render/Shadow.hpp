#pragma once
#include "FrameBuffer.hpp"
#include "Transform.hpp"

namespace SGE::Render{

struct ShadowData{
    const FrameBuffer *depth{};
    Matrix4DBase<double> lightViewProj{};
    double bias{0.005};
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

}
