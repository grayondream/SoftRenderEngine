#pragma once
#include "GeoObject/Color.hpp"
#include <cmath>

namespace SGE::Render{

// Analytic environment probe: gradient sky above horizon, procedural grid floor
// below. Stands in for a cube-map asset so the showcase needs no external data.
inline Vector3DBase<double> EnvNormalize(const Vector3DBase<double> &v){
    const double len = std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    if(len < 1e-12) return Vector3DBase<double>{0,0,1};
    return Vector3DBase<double>{v.x/len, v.y/len, v.z/len};
}

inline Color32 SampleEnvironment(const Vector3DBase<double> &dir){
    const auto d = EnvNormalize(dir);
    if(d.y >= 0.0){
        const double t = d.y;
        const int r = static_cast<int>(40 + 60 * t);
        const int g = static_cast<int>(80 + 120 * t);
        const int b = static_cast<int>(160 + 90 * t);
        return Color32{std::min(r,255), std::min(g,255), std::min(b,255), 255};
    }
    // grid floor
    const double fx = d.x / (-d.y), fz = d.z / (-d.y);
    const double gx = fx * 6.0, gz = fz * 6.0;
    const bool line = (std::abs(gx - std::floor(gx)) < 0.06) ||
                      (std::abs(gz - std::floor(gz)) < 0.06);
    if(line) return Color32{200, 200, 210, 255};
    const int shade = ((static_cast<int>(std::floor(gx)) + static_cast<int>(std::floor(gz))) % 2 == 0)
                    ? 70 : 110;
    return Color32{shade, shade + 10, shade + 20, 255};
}

struct EnvParams{
    bool enabled{false};
    double reflectivity{0.0};   // 0..1 mirror strength
    double refractivity{0.0};   // 0..1 transmission strength (glass)
    double ior{1.45};
};

}
