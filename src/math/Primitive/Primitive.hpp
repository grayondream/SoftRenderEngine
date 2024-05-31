#pragma once
#include "PrimitiveLine.hpp"
#include "PrimitivePlane.hpp"

using Line2D = PrimitiveLineBase<double, Vector2DBase<double>>;
using Line3D = PrimitiveLineBase<double, Vector3DBase<double>>;
using Line4D = PrimitiveLineBase<double, Vector4DBase<double>>;

using Plane3D = PrimitivePlaneBase<double>;
using Plane4D = PrimitivePlaneBase<double, Vector4DBase>;

