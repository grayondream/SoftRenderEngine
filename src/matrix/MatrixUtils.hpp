#pragma once
#include "Matrix.hpp"
#include <string>
#include <cassert>

namespace std{
    std::string to_string(const Matrix1D &mat);
    std::string to_string(const Matrix2D &mat);
    std::string to_string(const Matrix3D &mat);
    std::string to_string(const Matrix4D &mat);
}
