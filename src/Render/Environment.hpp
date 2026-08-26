#pragma once
#include "GeoObject/Color.hpp"
#include "Texture.hpp"
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

// Sample a 6-face cubemap along a direction (right,left,top,bottom,
// front,back), OpenGL cubemap convention.
inline Color32 SampleCubemap(const Vector3DBase<double> &dir,
                             const Texture *const *faces){
    const auto d = EnvNormalize(dir);
    const double ax = std::abs(d.x), ay = std::abs(d.y), az = std::abs(d.z);
    int face; double u, v;
    if(ax >= ay && ax >= az){
        face = d.x > 0 ? 0 : 1;
        u = -d.z / ax; v = -d.y / ax;
        if(d.x < 0){ u = -u; }
    }else if(ay >= az){
        face = d.y > 0 ? 2 : 3;
        u = d.x / ay; v = d.z / ay;
    }else{
        face = d.z > 0 ? 4 : 5;
        u = d.x / az; v = -d.y / az;
        if(d.z < 0){ u = -u; }
    }
    u = std::clamp(u * 0.5 + 0.5, 0.0, 0.99999);
    v = std::clamp(v * 0.5 + 0.5, 0.0, 0.99999);
    if(face < 0 || face >= 6 || !faces[face]){
        return Color32{0, 0, 0, 255};
    }
    const uint32_t c = faces[face]->sample(u, v, TextureFilter::Bilinear,
                                           TextureWrap::Clamp);
    return Color32{static_cast<int32_t>((c >> 16) & 0xFF),
                   static_cast<int32_t>((c >> 8) & 0xFF),
                   static_cast<int32_t>(c & 0xFF), 255};
}

struct EnvParams{
    bool enabled{false};
    double reflectivity{0.0};   // 0..1 mirror strength
    double refractivity{0.0};   // 0..1 transmission strength (glass)
    double ior{1.45};
    // optional 6-face cubemap (right,left,top,bottom,front,back). When
    // set, reflection/refraction sample it instead of the procedural env.
    const Texture *cubemapFaces[6]{};

    Color32 sampleDir(const Vector3DBase<double> &dir) const{
        bool hasCube = cubemapFaces[0] && cubemapFaces[1]
            && cubemapFaces[2] && cubemapFaces[3]
            && cubemapFaces[4] && cubemapFaces[5];
        if(hasCube){
            return SampleCubemap(dir, cubemapFaces);
        }
        return SampleEnvironment(dir);
    }
};

}
