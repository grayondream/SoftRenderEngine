#pragma once
#include <vector>
#include "Rasterizer.hpp"
#include "GeoObject/Object4D.hpp"
#include "../math/Matrix/DynamicMatrix4D.hpp"

namespace Pipeline{

struct ScreenTriangle{
    ScreenVertex v[3];
};

bool isBackFacing(const ScreenVertex &a, const ScreenVertex &b, const ScreenVertex &c);
std::vector<ScreenTriangle> clipNearPlane(const ScreenVertex (&tri)[3]);
std::vector<ScreenTriangle> projectObject(const Object4D &obj,
                                          const Matrix4DBase<double> &mvp,
                                          std::size_t screenW, std::size_t screenH);

}
