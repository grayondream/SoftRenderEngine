#pragma once

#include "DynamicMatrix.hpp"
#include "StaticMatrix.hpp"
#include "StaticMatrix.hpp"

using Matrix1D = Matrix1DBase<double>;
using Matrix2D = Matrix2DBase<double>;
using Matrix3D = Matrix3DBase<double>;
using Matrix4D = Matrix4DBase<double>;

using Matrix1x4 = StaticMatrix1DBase<double, 4>;
using Matrix1x3 = StaticMatrix1DBase<double, 3>;
using Matrix1x2 = StaticMatrix1DBase<double, 2>;

using Matrix4x1 = StaticMatrix2DBase<double, 4, 1>;
using Matrix3x1 = StaticMatrix2DBase<double, 3, 1>;
using Matrix2x1 = StaticMatrix2DBase<double, 2, 1>;

template<class T, StaticMatrixSizeType sz>
using StaticMarixSquare2D = StaticMatrix2DBase<T, sz, sz>;

template<class T, StaticMatrixSizeType sz>
using StaticMarixSquare3D = StaticMatrix3DBase<T, sz, sz, sz>;

template<class T, StaticMatrixSizeType sz>
using StaticMarixSquare4D = StaticMatrix4DBase<T, sz, sz, sz, sz>;

using Matrix4x4 = StaticMarixSquare2D<double, 4>;
using Matrix3x3 = StaticMarixSquare2D<double, 3>;
using Matrix2x2 = StaticMarixSquare2D<double, 2>;

using Matrix2x2x2 = StaticMarixSquare3D<double, 2>;
using Matrix3x3x3 = StaticMarixSquare3D<double, 3>;
using Matrix4x4x4 = StaticMarixSquare3D<double, 4>;

using Matrix2x2x2x2 = StaticMarixSquare4D<double, 2>;
using Matrix3x3x3x3 = StaticMarixSquare4D<double, 3>;
using Matrix4x4x4x4 = StaticMarixSquare4D<double, 4>;