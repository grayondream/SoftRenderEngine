#include "Light.hpp"
#include <algorithm>
#include "Environment.hpp"
#include <cmath>

namespace{
Vector3DBase<double> Sub(const Vector3DBase<double> &a, const Vector3DBase<double> &b){
    return Vector3DBase<double>{a.x - b.x, a.y - b.y, a.z - b.z};
}

Vector3DBase<double> Scale(const Vector3DBase<double> &a, double s){
    return Vector3DBase<double>{a.x * s, a.y * s, a.z * s};
}

double Clamp01(double v){
    return v < 0 ? 0 : (v > 1 ? 1 : v);
}
}

uint32_t shade(const LightingRig &rig,
               const Color32 &albedo,
               const Vector3DBase<double> &Nn,
               const Vector3DBase<double> &P,
               const Vector3DBase<double> &viewPos,
               double shadowFactor){
    const Vector3DBase<double> N = Nn.normalize();
    const Vector3DBase<double> V = Sub(viewPos, P).normalize();

    double diffR = rig.ambient, diffG = rig.ambient, diffB = rig.ambient;
    double specR = 0, specG = 0, specB = 0;

    auto accum = [&](const Vector3DBase<double> &L, const ColorFlt &lc, double atten){
        const double ndotl = std::max(0.0, N.dot(L));
        diffR += lc.r * ndotl * atten * shadowFactor;
        diffG += lc.g * ndotl * atten * shadowFactor;
        diffB += lc.b * ndotl * atten * shadowFactor;

        const Vector3DBase<double> Hv = Vector3DBase<double>{
            L.x + V.x, L.y + V.y, L.z + V.z}.normalize();
        const double sp = std::pow(std::max(0.0, N.dot(Hv)),
                                   static_cast<double>(rig.shininess))
                        * rig.specularStrength * atten * shadowFactor;
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

    for(const auto &sl : rig.spot){
        const Vector3DBase<double> diff = Sub(sl.position, P);
        const double dist = diff.length();
        if(dist > sl.range || dist < 1e-9) continue;
        const double atten = Clamp01(1.0 - dist / sl.range);
        if(atten <= 0) continue;
        const Vector3DBase<double> L = Scale(diff, 1.0 / dist);
        const double cosAng = L.dot(Scale(sl.direction.normalize(), -1.0));
        if(cosAng < sl.cutoffCos) continue;
        const double edge = Clamp01((cosAng - sl.cutoffCos) / std::max(1e-4, 1.0 - sl.cutoffCos));
        accum(L, sl.color, atten * edge * edge);
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

namespace{
double D_GGX(double NoH, double a){
    const double a2 = a * a;
    const double dd = NoH * NoH * (a2 - 1.0) + 1.0;
    return a2 / std::max(1e-9, 3.14159265358979 * dd * dd);
}

double G_Smith(double NoV, double NoL, double a){
    const double k = (a + 1.0) * (a + 1.0) / 8.0;
    const double gv = NoV / (NoV * (1.0 - k) + k);
    const double gl = NoL / (NoL * (1.0 - k) + k);
    return gv * gl;
}
}

uint32_t pbrShade(const LightingRig &rig,
                  const PbrMaterial &mat,
                  const Vector3DBase<double> &Nn,
                  const Vector3DBase<double> &P,
                  const Vector3DBase<double> &viewPos,
                  double shadowFactor){
    const Vector3DBase<double> N = Nn.normalize();
    const Vector3DBase<double> V = Sub(viewPos, P).normalize();
    const double NoV = std::max(1e-4, N.dot(V));
    const double a = std::max(0.045, static_cast<double>(mat.roughness) * mat.roughness);
    double specR = 0, specG = 0, specB = 0;
    double lr = 0, lg = 0, lb = 0;
    for(const auto &dl : rig.directional){
        const Vector3DBase<double> L = Scale(dl.direction.normalize(), -1.0);
        const auto Hv = Vector3DBase<double>{L.x+V.x, L.y+V.y, L.z+V.z}.normalize();
        const double NoL = std::max(0.0, N.dot(L));
        const double NoH = std::max(0.0, N.dot(Hv));
        const double VoH = std::max(0.0, V.dot(Hv));
        if(NoL <= 0) continue;

        const double F0m = 0.04;
        double Fr = F0m + (mat.baseColor.r / 255.0 - F0m) * mat.metallic;
        double Fg = F0m + (mat.baseColor.g / 255.0 - F0m) * mat.metallic;
        double Fb = F0m + (mat.baseColor.b / 255.0 - F0m) * mat.metallic;

        const double D = D_GGX(NoH, a);
        const double G = G_Smith(NoV, NoL, a);
        specR += dl.color.r * shadowFactor * D * G / std::max(1e-6, 4.0 * NoV * NoL)
               * (Fr * VoH + 0.05);
        specG += dl.color.g * shadowFactor * D * G / std::max(1e-6, 4.0 * NoV * NoL)
               * (Fg * VoH + 0.05);
        specB += dl.color.b * shadowFactor * D * G / std::max(1e-6, 4.0 * NoV * NoL)
               * (Fb * VoH + 0.05);

        const double kd = (1.0 - mat.metallic) * (1.0 - Fr);
        lr += dl.color.r * kd * NoL * shadowFactor;
        lg += dl.color.g * kd * NoL * shadowFactor;
        lb += dl.color.b * kd * NoL * shadowFactor;
    }

    // point lights: radiance = color / d^2 (reference PBR.fs)
    for(const auto &pl : rig.point){
        const Vector3DBase<double> Lv{pl.position.x - P.x,
            pl.position.y - P.y, pl.position.z - P.z};
        const double dist2 = Lv.dot(Lv);
        if(dist2 < 1e-6 || dist2 > pl.range * pl.range) continue;
        const double dist = std::sqrt(dist2);
        const Vector3DBase<double> L = Scale(Lv, 1.0 / dist);
        const double NoL = std::max(0.0, N.dot(L));
        if(NoL <= 0) continue;
        const auto Hv = Vector3DBase<double>{L.x+V.x, L.y+V.y,
            L.z+V.z}.normalize();
        const double NoH = std::max(0.0, N.dot(Hv));
        const double VoH = std::max(0.0, V.dot(Hv));

        const double F0m = 0.04;
        double Fr = F0m + (mat.baseColor.r / 255.0 - F0m) * mat.metallic;
        double Fg = F0m + (mat.baseColor.g / 255.0 - F0m) * mat.metallic;
        double Fb = F0m + (mat.baseColor.b / 255.0 - F0m) * mat.metallic;

        const double D = D_GGX(NoH, a);
        const double G = G_Smith(NoV, NoL, a);
        // Schlick fresnel with VoH (reference fresnelSchlick)
                Fr = F0m + (Fr - F0m) * std::pow(1.0 - VoH, 5.0);
        Fg = F0m + (Fg - F0m) * std::pow(1.0 - VoH, 5.0);
        Fb = F0m + (Fb - F0m) * std::pow(1.0 - VoH, 5.0);
        const double radiance =
            static_cast<double>(pl.color.r + pl.color.g + pl.color.b)
            / 3.0 / dist2;
        specR += radiance * shadowFactor * D * G
               / std::max(1e-6, 4.0 * NoV * NoL) * Fr;
        specG += radiance * shadowFactor * D * G
               / std::max(1e-6, 4.0 * NoV * NoL) * Fg;
        specB += radiance * shadowFactor * D * G
               / std::max(1e-6, 4.0 * NoV * NoL) * Fb;
        const double kd = (1.0 - mat.metallic) * (1.0 - Fr);
        lr += pl.color.r / 255.0 * radiance * kd * NoL * shadowFactor;
        lg += pl.color.g / 255.0 * radiance * kd * NoL * shadowFactor;
        lb += pl.color.b / 255.0 * radiance * kd * NoL * shadowFactor;
    }

    // analytic image-based approximation: hemispheric irradiance + roughened specular
    const Color32 skyN = SGE::Render::SampleEnvironment(N);
    const double ambKd = (1.0 - mat.metallic);
    lr += skyN.r / 255.0 * ambKd * rig.ambient;
    lg += skyN.g / 255.0 * ambKd * rig.ambient;
    lb += skyN.b / 255.0 * ambKd * rig.ambient;
    specR += skyN.r / 255.0 * mat.metallic * (1.0 - a) * rig.ambient;
    specG += skyN.g / 255.0 * mat.metallic * (1.0 - a) * rig.ambient;
    specB += skyN.b / 255.0 * mat.metallic * (1.0 - a) * rig.ambient;

    // reference: linear HDR-ish sum then Reinhard tonemap + gamma 2.2
    auto chFinal = [&](int baseC, double lightSum, double specSum){
        double lin = baseC / 255.0 * lightSum + specSum;
        lin = lin / (lin + 1.0);                       // Reinhard
        lin = std::pow(std::clamp(lin, 0.0, 1.0), 1.0 / 2.2);
        return static_cast<uint32_t>(std::min(
            255.0, std::max(0.0, lin * 255.0)));
    };
    const uint32_t outR = chFinal(mat.baseColor.r, lr, specR);
    const uint32_t outG = chFinal(mat.baseColor.g, lg, specG);
    const uint32_t outB = chFinal(mat.baseColor.b, lb, specB);
    return 0xFF000000u | (std::min(outR, 255u) << 16) |
           (std::min(outG, 255u) << 8) | std::min(outB, 255u);
}
