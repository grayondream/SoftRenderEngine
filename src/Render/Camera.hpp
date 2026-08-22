#pragma once
#include "Transform.hpp"

namespace SGE::Render{

struct InputState{
    bool w{}, a{}, s{}, d{};
    bool r{}, f{};
    bool left{}, right{};
    bool up{}, down{};
};

class Camera{
public:
    Vector3DBase<double> position{0, 2, -6};
    double yaw{0.0};
    double pitch{-0.3217505543966422};

    Vector3DBase<double> forward() const;
    Vector3DBase<double> right() const;
    Matrix4DBase<double> viewMatrix() const;
};

void update(Camera &cam, const InputState &in, double dt);

}
