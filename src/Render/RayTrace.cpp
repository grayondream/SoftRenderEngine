#include "RayTrace.hpp"
#include "Transform.hpp"
#include "Rasterizer.hpp"
#include <cmath>
#include <algorithm>
#include <limits>

namespace SGE::Render{

namespace{

constexpr double kEps = 1e-3;

bool RayAABBIntersect(const Vector3DBase<double> &origin,
                      const Vector3DBase<double> &dir,
                      const Vector3DBase<double> &bmin,
                      const Vector3DBase<double> &bmax,
                      double &tmin, double &tmax){
    tmin = -std::numeric_limits<double>::infinity();
    tmax = std::numeric_limits<double>::infinity();
    for(int i = 0; i < 3; i++){
        const double orig = (i == 0 ? origin.x : (i == 1 ? origin.y : origin.z));
        const double d = (i == 0 ? dir.x : (i == 1 ? dir.y : dir.z));
        const double b0 = (i == 0 ? bmin.x : (i == 1 ? bmin.y : bmin.z));
        const double b1 = (i == 0 ? bmax.x : (i == 1 ? bmax.y : bmax.z));
        if(std::abs(d) < 1e-12){
            if(orig < b0 || orig > b1) return false;
        }else{
            const double invD = 1.0 / d;
            double t0 = (b0 - orig) * invD;
            double t1 = (b1 - orig) * invD;
            if(t0 > t1) std::swap(t0, t1);
            tmin = std::max(tmin, t0);
            tmax = std::min(tmax, t1);
            if(tmin > tmax) return false;
        }
    }
    return true;
}

double SchlickFresnel(double cosTheta, double iorRatio){
    const double r0 = (1.0 - iorRatio) / (1.0 + iorRatio);
    const double r02 = r0 * r0;
    const double t = 1.0 - cosTheta;
    return r02 + (1.0 - r02) * t * t * t * t * t;
}

Vector3DBase<double> Sub(const Vector3DBase<double> &a, const Vector3DBase<double> &b){
    return Vector3DBase<double>{a.x - b.x, a.y - b.y, a.z - b.z};
}
Vector3DBase<double> Scale(const Vector3DBase<double> &a, double s){
    return Vector3DBase<double>{a.x * s, a.y * s, a.z * s};
}
double DotP(const Vector3DBase<double> &a, const Vector3DBase<double> &b){
    return a.dot(b);
}
double Length(const Vector3DBase<double> &a){
    return std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}
Vector3DBase<double> Normalize(const Vector3DBase<double> &a){
    const double len = Length(a);
    if(len < 1e-12) return Vector3DBase<double>{0, 0, 0};
    return Scale(a, 1.0 / len);
}
double DynamicEps(const Vector3DBase<double> &point, double scale = 1.0){
    const double mag = std::max({std::abs(point.x), std::abs(point.y), std::abs(point.z)});
    return kEps * scale * std::max(1.0, mag);
}
Vector3DBase<double> Add(const Vector3DBase<double> &a, const Vector3DBase<double> &b){
    return Vector3DBase<double>{a.x + b.x, a.y + b.y, a.z + b.z};
}
bool Refract(const Vector3DBase<double> &v, const Vector3DBase<double> &n,
             double eta, Vector3DBase<double> &out){
    const double cosI = -(DotP(v, n));
    const double k = 1.0 - eta * eta * (1.0 - cosI * cosI);
    if(k < 0) return false;
    out = Add(Scale(v, eta), Scale(n, eta * cosI - std::sqrt(k)));
    return true;
}

}

namespace RayDetail{

bool raySphereIntersect(const RaySphere &s,
                        const Vector3DBase<double> &origin,
                        const Vector3DBase<double> &dir, double &outDist){
    const Vector3DBase<double> oc = Sub(origin, s.center);
    const double b = DotP(oc, dir);
    const double c = DotP(oc, oc) - s.radius * s.radius;
    const double disc = b * b - c;
    if(disc < 0) return false;
    const double sq = std::sqrt(disc);
    double t = -b - sq;
    if(t < kEps){
        t = -b + sq;
        if(t < kEps) return false;
    }
    outDist = t;
    return true;
}

bool rayTriangleIntersect(const RayTriangle &tri,
                          const Vector3DBase<double> &origin,
                          const Vector3DBase<double> &dir, double &outDist){
    const Vector3DBase<double> e1 = Sub(tri.b, tri.a);
    const Vector3DBase<double> e2 = Sub(tri.c, tri.a);
    const Vector3DBase<double> p = dir.mul(e2);
    const double det = DotP(e1, p);
    if(std::abs(det) < kEps) return false;
    const double invDet = 1.0 / det;
    const Vector3DBase<double> tv = Sub(origin, tri.a);
    const double u = DotP(tv, p) * invDet;
    if(u < -kEps || u > 1.0 + kEps) return false;
    const Vector3DBase<double> q = tv.mul(e1);
    const double v = DotP(dir, q) * invDet;
    if(v < -kEps || u + v > 1.0 + kEps) return false;
    const double t = DotP(e2, q) * invDet;
    if(t < kEps) return false;
    outDist = t;
    return true;
}

}

namespace{

constexpr double kFovY = 3.14159265358979323846 / 3;

RayDetail::Hit SphereHit(const RaySphere &s,
                         const Vector3DBase<double> &origin,
                         const Vector3DBase<double> &dir, double dist){
    RayDetail::Hit h{};
    h.hit = true;
    h.dist = dist;
    h.point = Add(origin, Scale(dir, dist));
    h.normal = Scale(Sub(h.point, s.center), 1.0 / s.radius);
    h.albedo = s.albedo;
    h.reflectivity = s.reflectivity;
    h.refractivity = s.refractivity;
    h.ior = s.ior;
    return h;
}

RayDetail::Hit TriangleHit(const RayTriangle &tri,
                           const Vector3DBase<double> &origin,
                           const Vector3DBase<double> &dir, double dist){
    RayDetail::Hit h{};
    h.hit = true;
    h.dist = dist;
    h.point = Add(origin, Scale(dir, dist));
    auto n = Sub(tri.b, tri.a).mul(Sub(tri.c, tri.a));
    const double len = n.length();
    if(len > kEps) n = n.normalize();
    if(DotP(n, dir) > 0) n = Scale(n, -1.0);
    h.normal = n;
    h.albedo = tri.albedo;
    h.reflectivity = tri.reflectivity;
    return h;
}

uint32_t MixChannels(uint32_t src, uint32_t dst, float r){
    auto mix = [r](uint32_t s, uint32_t d){
        const double v = s * (1.0 - r) + d * static_cast<double>(r);
        return static_cast<uint32_t>(v + 0.5);
    };
    const uint32_t outR = mix((src >> 16) & 0xFF, (dst >> 16) & 0xFF);
    const uint32_t outG = mix((src >> 8) & 0xFF, (dst >> 8) & 0xFF);
    const uint32_t outB = mix(src & 0xFF, dst & 0xFF);
    return 0xFF000000u | (outR << 16) | (outG << 8) | outB;
}

}

RayDetail::Hit RayTracer::traceRay(const RayScene &scene,
                                   const Vector3DBase<double> &origin,
                                   const Vector3DBase<double> &dir) const{
    if(scene.hasBBox){
        double tmin, tmax;
        if(!RayAABBIntersect(origin, dir, scene.bboxMin, scene.bboxMax, tmin, tmax)){
            return RayDetail::Hit{};
        }
    }

    RayDetail::Hit best{};
    double bestDist = std::numeric_limits<double>::infinity();
    for(const auto &s : scene.spheres){
        double t = 0;
        if(RayDetail::raySphereIntersect(s, origin, dir, t) && t < bestDist){
            bestDist = t;
            best = SphereHit(s, origin, dir, t);
        }
    }
    for(const auto &tri : scene.triangles){
        double t = 0;
        if(RayDetail::rayTriangleIntersect(tri, origin, dir, t) && t < bestDist){
            bestDist = t;
            best = TriangleHit(tri, origin, dir, t);
        }
    }
    return best;
}

bool RayTracer::occluded(const RayScene &scene,
                         const Vector3DBase<double> &origin,
                         const Vector3DBase<double> &dir, double maxDist) const{
    for(const auto &s : scene.spheres){
        double t = 0;
        if(RayDetail::raySphereIntersect(s, origin, dir, t) && t < maxDist - kEps) return true;
    }
    for(const auto &tri : scene.triangles){
        double t = 0;
        if(RayDetail::rayTriangleIntersect(tri, origin, dir, t) && t < maxDist - kEps) return true;
    }
    return false;
}

uint32_t RayTracer::shadeHit(const RayScene &scene, const LightingRig &rig,
                             const RayDetail::Hit &hit,
                             const Vector3DBase<double> &viewPos, int depth,
                             const RayTraceOptions &opt){
    const auto V = Sub(viewPos, hit.point).normalize();
    const auto N = hit.normal.normalize();

    double diffR = rig.ambient * rig.ambientColor.r;
    double diffG = rig.ambient * rig.ambientColor.g;
    double diffB = rig.ambient * rig.ambientColor.b;
    double specR = 0, specG = 0, specB = 0;

    auto accumLight = [&](const Vector3DBase<double> &L, const ColorFlt &lc, double atten, double shadow){
        const double ndotl = std::max(0.0, N.dot(L));
        diffR += lc.r * ndotl * atten * shadow;
        diffG += lc.g * ndotl * atten * shadow;
        diffB += lc.b * ndotl * atten * shadow;

        double sp = 0.0;
        if(rig.blinnPhong){
            const Vector3DBase<double> Hv = Vector3DBase<double>{
                L.x + V.x, L.y + V.y, L.z + V.z}.normalize();
            sp = std::pow(std::max(0.0, N.dot(Hv)),
                          static_cast<double>(rig.shininess));
        }else{
            const double ndl2 = 2.0 * N.dot(L);
            const Vector3DBase<double> R{
                ndl2 * N.x - L.x, ndl2 * N.y - L.y, ndl2 * N.z - L.z};
            sp = std::pow(std::max(0.0, R.dot(V)),
                          static_cast<double>(rig.shininess));
        }
        sp *= rig.specularStrength * atten * shadow;
        specR += lc.r * sp;
        specG += lc.g * sp;
        specB += lc.b * sp;
    };

    for(const auto &dl : rig.directional){
        const auto L = Scale(dl.direction.normalize(), -1.0);
        const double eps = DynamicEps(hit.point);
        double shadow = occluded(scene, Add(hit.point, Scale(L, eps)), L,
                                 std::numeric_limits<double>::infinity()) ? 0.0 : 1.0;
        accumLight(L, dl.color, 1.0, shadow);
    }
    for(const auto &pl : rig.point){
        const auto diff = Sub(pl.position, hit.point);
        const double dist = diff.length();
        if(dist < kEps) continue;
        const auto L = Scale(diff, 1.0 / dist);
        const double eps = DynamicEps(hit.point, dist);
        double atten;
        if(pl.quadratic > 0 || pl.linear > 0){
            atten = 1.0 / (1.0 + pl.linear * dist + pl.quadratic * dist * dist);
        }else{
            if(pl.range <= 0) continue;
            atten = std::max(0.0, 1.0 - dist / pl.range);
        }
        if(atten <= 0) continue;
        double shadow = occluded(scene, Add(hit.point, Scale(L, eps)), L, dist) ? 0.0 : 1.0;
        accumLight(L, pl.color, atten, shadow);
    }

    auto ch = [](double albedoC, double lightSum, double specSum) -> uint32_t {
        double s = albedoC / 255.0 * lightSum + specSum;
        s *= 255.0;
        if(!(s >= 0)) s = 0;
        else if(s > 255) s = 255;
        return static_cast<uint32_t>(s);
    };
    uint32_t local = (255u << 24)
         | (ch(hit.albedo.r, diffR, specR) << 16)
         | (ch(hit.albedo.g, diffG, specG) << 8)
         |  ch(hit.albedo.b, diffB, specB);

    if(hit.refractivity > 0.0f){
        const float rr = std::clamp(hit.refractivity, 0.0f, 1.0f);
        if(rr > 0.0f && depth + 1 <= opt.maxDepth){
            const auto D = Sub(hit.point, viewPos).normalize();
            const bool entering = DotP(D, hit.normal) < 0;
            const double cosI = std::abs(DotP(D, hit.normal));
            const double eta = entering ? (1.0 / hit.ior) : hit.ior;
            const double etaRatio = entering ? hit.ior : (1.0 / hit.ior);
            const double fresnel = SchlickFresnel(cosI, etaRatio);

            Vector3DBase<double> rf{};
            bool totalInternalReflection = !Refract(D, hit.normal, eta, rf);
            const double eps = DynamicEps(hit.point);

            if(totalInternalReflection || fresnel > 0.5){
                const auto R = Sub(D, Scale(hit.normal, 2.0 * DotP(D, hit.normal)));
                const RayDetail::Hit reflected = traceRay(scene,
                    Add(hit.point, Scale(R, eps)), R);
                uint32_t reflColor = PackBGRA(opt.background);
                if(reflected.hit){
                    reflColor = shadeHit(scene, rig, reflected,
                        Add(hit.point, Scale(R, eps)), depth + 1, opt);
                }
                local = MixChannels(local, reflColor, fresnel);
            }else{
                const RayDetail::Hit inner = traceRay(scene,
                    Add(hit.point, Scale(rf, eps)), rf);
                uint32_t refrColor = PackBGRA(opt.background);
                if(inner.hit){
                    refrColor = shadeHit(scene, rig, inner,
                        Add(hit.point, Scale(rf, eps)), depth + 1, opt);
                }
                local = MixChannels(local, refrColor, rr * (1.0 - fresnel));
            }
        }
    }

    const float r = std::clamp(hit.reflectivity, 0.0f, 1.0f);
    if(r <= 0.0f || depth + 1 > opt.maxDepth){
        return local;
    }

    const auto D = Sub(hit.point, viewPos).normalize();
    const auto R = Sub(D, Scale(hit.normal, 2.0 * DotP(D, hit.normal)));
    const double eps = DynamicEps(hit.point);
    const RayDetail::Hit reflected = traceRay(scene,
        Add(hit.point, Scale(R, eps)), R);

    uint32_t reflColor;
    if(reflected.hit){
        reflColor = shadeHit(scene, rig, reflected,
                             Add(hit.point, Scale(R, eps)), depth + 1, opt);
    }else{
        reflColor = PackBGRA(opt.background);
    }
    return MixChannels(local, reflColor, r);
}

void RayTracer::render(const RayScene &scene, const Camera &camera,
                       const LightingRig &rig, const RayTraceOptions &opt){
    const std::size_t W = m_fb.width();
    const std::size_t H = m_fb.height();
    const double aspect = static_cast<double>(W) / static_cast<double>(H);
    const double halfH = std::tan(kFovY * 0.5);
    const double halfW = halfH * aspect;

    const auto fwd = camera.forward().normalize();
    auto right = fwd.mul(Vector3DBase<double>{0, 1, 0}).normalize();
    const auto up = right.mul(fwd).normalize();

    const int scale = std::max(1, opt.renderScale);

    for(std::size_t py = 0; py < H; py += scale){
        for(std::size_t px = 0; px < W; px += scale){
            const double sx = (static_cast<double>(px) + 0.5) / W * 2.0 - 1.0;
            const double sy = 1.0 - (static_cast<double>(py) + 0.5) / H * 2.0;
            auto dir = Add(Add(Scale(right, sx * halfW), Scale(up, sy * halfH)), fwd);
            dir = dir.normalize();

            const RayDetail::Hit hit = traceRay(scene, camera.position, dir);
            uint32_t color;
            if(!hit.hit){
                color = PackBGRA(opt.background);
            }else{
                color = shadeHit(scene, rig, hit, camera.position, 0, opt);
            }

            if(scene.cone.enabled){
                double coneAccum = 0.0;
                constexpr int kSteps = 8;
                const double maxT = hit.hit ? std::min(hit.dist, scene.cone.range) : scene.cone.range;
                const double stepLen = maxT / kSteps;
                for(int st = 1; st <= kSteps; st++){
                    const double t = st * stepLen;
                    const auto sp = Add(camera.position, Scale(dir, t));
                    const auto rel = Sub(sp, scene.cone.position);
                    const double dist = Length(rel);
                    if(dist < 0.1 || dist > scene.cone.range) continue;
                    const auto dirToSample = Normalize(rel);
                    const double cosA = DotP(dirToSample, Normalize(scene.cone.direction));
                    if(cosA < scene.cone.cutoffCos) continue;
                    const double edgeFade = (cosA - scene.cone.cutoffCos)
                                / std::max(1e-4, 1.0 - scene.cone.cutoffCos);
                    const double distFade = 1.0 - (dist / scene.cone.range);
                    const double density = edgeFade * distFade * distFade;
                    coneAccum += density * stepLen;
                }
                coneAccum *= 0.04 * scene.cone.intensity;
                coneAccum = std::min(coneAccum, 1.0);
                if(coneAccum > 0.001){
                    const double warmR = 1.0;
                    const double warmG = 0.93;
                    const double warmB = 0.85;
                    auto mixIn = [&](uint32_t baseC){
                        auto ch2 = [&](int shift, double tint){
                            const double b = (baseC >> shift) & 0xFF;
                            const double lit = b + (255 * tint - b) * coneAccum;
                            return static_cast<uint32_t>(std::min(255.0, std::max(0.0, lit + 0.5))) << shift;
                        };
                        return 0xFF000000u | ch2(16, warmR) | ch2(8, warmG) | ch2(0, warmB);
                    };
                    color = mixIn(color);
                }
            }

            for(std::size_t dy = 0; dy < static_cast<std::size_t>(scale) && py + dy < H; dy++){
                for(std::size_t dx = 0; dx < static_cast<std::size_t>(scale) && px + dx < W; dx++){
                    m_fb.setPixelOverlay(px + dx, py + dy, color);
                }
            }
        }
    }
}

}
