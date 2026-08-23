#include "Light.hpp"
#include <algorithm>
#include <cmath>

namespace{
Vector3DBase<double> Sub(const Vector3DBase<double> &a, const Vector3DBase<double> &b){
    return Vector3DBase<double>{a.x - b.x, a.y - b.y, a.z - b.z};
}

double Clamp01(double v){
    return v < 0 ? 0 : (v > 1 ? 1 : v);
}
}

uint32_t shade(const LightingRig &rig,
               const Color32 &albedo,
               const Vector3DBase<double> &Nn,
               const Vector3DBase<double> &P,
               const Vector3DBase<double> &viewPos){
    const Vector3DBase<double> N = Nn.normalize();
    const Vector3DBase<double> V = Sub(viewPos, P).normalize();

    double diffR = rig.ambient, diffG = rig.ambient, diffB = rig.ambient;
    double specR = 0, specG = 0, specB = 0;

    auto accum = [&](const Vector3DBase<double> &L, const ColorFlt &lc, double atten){
        const double ndotl = std::max(0.0, N.dot(L));
        diffR += lc.r * ndotl * atten;
        diffG += lc.g * ndotl * atten;
        diffB += lc.b * ndotl * atten;

        const Vector3DBase<double> Hv = Vector3DBase<double>{
            L.x + V.x, L.y + V.y, L.z + V.z}.normalize();
        const double sp = std::pow(std::max(0.0, N.dot(Hv)),
                                   static_cast<double>(rig.shininess))
                        * rig.specularStrength * atten;
        specR += lc.r * sp;
        specG += lc.g * sp;
        specB += lc.b * sp;
    };

    for(const auto &dl : rig.directional){
        accum(dl.direction.normalize(), dl.color, 1.0);
    }
    for(const auto &pl : rig.point){
        if(pl.range <= 0) continue;
        const Vector3DBase<double> diff = Sub(pl.position, P);
        const double atten = Clamp01(1.0 - diff.length() / pl.range);
        if(atten <= 0) continue;
        accum(diff.normalize(), pl.color, atten);
    }

    auto ch = [](double albedoC, double lightSum, double specSum) -> uint32_t {
        double s = albedoC / 255.0 * lightSum + specSum;
        s *= 255.0;
        if(!(s >= 0)) s = 0;
        else if(s > 255) s = 255;
        return static_cast<uint32_t>(s);
    };

    return (255u << 24)
         | (ch(albedo.r, diffR, specR) << 16)
         | (ch(albedo.g, diffG, specG) << 8)
         |  ch(albedo.b, diffB, specB);
}
