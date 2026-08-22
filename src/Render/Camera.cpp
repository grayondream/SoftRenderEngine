#include "Camera.hpp"
#include <algorithm>
#include <cmath>

namespace SGE::Render{

namespace{
constexpr double kMoveSpeed = 3.0;
constexpr double kTurnSpeed = 1.5;
constexpr double kPitchLimit = 3.14159265358979323846 / 2 - 0.01;

Vector3DBase<double> Add(const Vector3DBase<double> &a, const Vector3DBase<double> &b){
    return Vector3DBase<double>{a.x + b.x, a.y + b.y, a.z + b.z};
}
}

Vector3DBase<double> Camera::forward() const{
    return Vector3DBase<double>{
        std::sin(yaw) * std::cos(pitch),
        std::sin(pitch),
        std::cos(yaw) * std::cos(pitch)};
}

Vector3DBase<double> Camera::right() const{
    return forward().mul(Vector3DBase<double>{0, 1, 0}).normalize();
}

Matrix4DBase<double> Camera::viewMatrix() const{
    return SGE::Math::lookAt(position,
                             Add(position, forward()),
                             Vector3DBase<double>{0, 1, 0});
}

void update(Camera &cam, const InputState &in, double dt){
    cam.yaw += ((in.right ? 1.0 : 0.0) - (in.left ? 1.0 : 0.0)) * kTurnSpeed * dt;
    cam.pitch += ((in.up ? 1.0 : 0.0) - (in.down ? 1.0 : 0.0)) * kTurnSpeed * dt;
    cam.pitch = std::clamp(cam.pitch, -kPitchLimit, kPitchLimit);

    if(in.w || in.s || in.a || in.d || in.r || in.f){
        const auto fwd = cam.forward();
        const auto rgt = cam.right();
        double dx = 0, dy = 0, dz = 0;
        if(in.w){ dx += fwd.x; dy += fwd.y; dz += fwd.z; }
        if(in.s){ dx -= fwd.x; dy -= fwd.y; dz -= fwd.z; }
        if(in.a){ dx -= rgt.x; dy -= rgt.y; dz -= rgt.z; }
        if(in.d){ dx += rgt.x; dy += rgt.y; dz += rgt.z; }
        if(in.r){ dy += 1.0; }
        if(in.f){ dy -= 1.0; }
        cam.position = Add(cam.position,
            Vector3DBase<double>{dx * kMoveSpeed * dt, dy * kMoveSpeed * dt, dz * kMoveSpeed * dt});
    }
}

}
