#include "RayTrace.hpp"
#include "Transform.hpp"
#include "Rasterizer.hpp"
#include <cmath>
#include <algorithm>

namespace SGE::Render{

namespace{

constexpr double kEps = 1e-9;

Vector3DBase<double> Sub(const Vector3DBase<double> &a, const Vector3DBase<double> &b){
    return Vector3DBase<double>{a.x - b.x, a.y - b.y, a.z - b.z};
}
Vector3DBase<double> Scale(const Vector3DBase<double> &a, double s){
    return Vector3DBase<double>{a.x * s, a.y * s, a.z * s};
}
double DotP(const Vector3DBase<double> &a, const Vector3DBase<double> &b){
    return a.dot(b);
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
    RayDetail::Hit best{};
    double bestDist = 1e300;
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
    double shadowFactor = 1.0;
    for(const auto &dl : rig.directional){
        const auto L = Scale(dl.direction.normalize(), -1.0);
        if(occluded(scene, Add(hit.point, Scale(L, kEps * 8)), L, 1e300)){
            shadowFactor = 0.0;
            break;
        }
    }

    uint32_t local = shade(rig, hit.albedo, hit.normal, hit.point, viewPos, shadowFactor);

    // caustic light spots on receivers from glass spheres under the cone
    uint32_t causticAdd = 0;
    {
        double cr = 0, cg = 0, cb = 0;
        for(const auto &gs : scene.spheres){
            if(gs.refractivity <= 0.0f) continue;
            const auto toBall = Sub(gs.center, scene.cone.position);
            if(DotP(SGE::Render::EnvNormalize(toBall),
                    SGE::Render::EnvNormalize(scene.cone.direction)) < scene.cone.cutoffCos)
                continue;
            const auto Ld = SGE::Render::EnvNormalize(scene.cone.direction);
            // project ball center along L onto the hit plane
            const double denom = DotP(Ld, hit.normal);
            if(std::abs(denom) < 1e-6) continue;
            const double tHit = DotP(Sub(gs.center, hit.point), hit.normal) / denom;
            if(tHit <= 0) continue;
            const auto focus = Sub(hit.point, Scale(Ld, -tHit));
            (void)focus;
            const auto rel = Sub(hit.point, gs.center);
            const double lateral = std::sqrt(std::max(0.0,
                DotP(rel, rel) - DotP(rel, Ld) * DotP(rel, Ld)));
            const double spotR = gs.radius * 0.9;
            if(lateral < spotR && DotP(rel, rel) > gs.radius * gs.radius){
                const double w = (1.0 - lateral / spotR) * gs.refractivity
                               * scene.cone.intensity * 1.4;
                cr += gs.albedo.r * 0.5 + 120.0 * w;
                cg += gs.albedo.g * 0.5 + 120.0 * w;
                cb += gs.albedo.b * 0.5 + 120.0 * w;
            }
        }
        if(cr > 0 || cg > 0 || cb > 0){
            auto clamp255 = [](double v){
                v *= 0.35;
                return static_cast<uint32_t>(std::min(255.0, v));
            };
            causticAdd = 0xFF000000u |
                (clamp255(cr) << 16) | (clamp255(cg) << 8) | clamp255(cb);
        }
    }
    if(causticAdd != 0xFF000000u){
        auto addC = [](uint32_t a, uint32_t b, int shift){
            const uint32_t v = ((a >> shift) & 0xFF) + ((b >> shift) & 0xFF);
            return std::min(255u, v) << shift;
        };
        local = 0xFF000000u | addC(local, causticAdd, 16) |
                addC(local, causticAdd, 8) | addC(local, causticAdd, 0);
    }

    // glass refraction
    if(hit.reflectivity <= 0.0f){
        for(const auto &gs : scene.spheres){
            if(gs.refractivity <= 0.0f) continue;
            const auto centerDelta = Sub(hit.point, gs.center);
            const double rad2 = gs.radius * gs.radius;
            if(std::abs(DotP(centerDelta, centerDelta) - rad2) < 1e-6){
                const float rr = std::clamp(gs.refractivity, 0.0f, 1.0f);
                if(rr > 0.0f && depth + 1 <= opt.maxDepth){
                    const auto D = Sub(hit.point, viewPos).normalize();
                    Vector3DBase<double> rf{};
                    if(Refract(D, hit.normal, 1.0 / gs.ior, rf)){
                        const RayDetail::Hit inner = traceRay(scene,
                            Add(hit.point, Scale(rf, kEps * 8)), rf);
                        uint32_t refrColor = PackBGRA(opt.background);
                        if(inner.hit){
                            refrColor = shadeHit(scene, rig, inner,
                                Add(hit.point, Scale(rf, kEps * 8)), depth + 1, opt);
                        }
                        auto mixCh2 = [&](int shift){
                            const double baseV = (local >> shift) & 0xFF;
                            const double tV = (refrColor >> shift) & 0xFF;
                            const double val = baseV * (1.0 - rr * 0.8) +
                                tV * (rr * 0.8);
                            return static_cast<uint32_t>(val + 0.5) << shift;
                        };
                        local = 0xFF000000u | mixCh2(16) |
                                mixCh2(8) | mixCh2(0);
                    }
                }
                break;
            }
        }
    }

    const float r = std::clamp(hit.reflectivity, 0.0f, 1.0f);
    if(r <= 0.0f || depth + 1 > opt.maxDepth){
        return local;
    }

    const auto D = Sub(hit.point, viewPos).normalize();
    const auto R = Sub(D, Scale(hit.normal, 2.0 * DotP(D, hit.normal)));
    const RayDetail::Hit reflected = traceRay(scene,
        Add(hit.point, Scale(R, kEps * 8)), R);

    uint32_t reflColor;
    if(reflected.hit){
        reflColor = shadeHit(scene, rig, reflected,
                             Add(hit.point, Scale(R, kEps * 8)), depth + 1, opt);
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

    for(std::size_t py = 0; py < H; py++){
        for(std::size_t px = 0; px < W; px++){
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
                constexpr int kSteps = 24;
                const double stepLen = scene.cone.range / kSteps;
                for(int st = 1; st <= kSteps; st++){
                    const double t = st * stepLen;
                    if(hit.hit && t > hit.dist) break;
                    const auto sp = Add(camera.position, Scale(dir, t));
                    const auto rel = Sub(sp, scene.cone.position);
                    const double dist = rel.length();
                    if(dist > scene.cone.range || dist < 0.3) continue;
                    const double cosA = DotP(Scale(rel, 1.0 / dist),
                        SGE::Render::EnvNormalize(scene.cone.direction));
                    if(cosA < scene.cone.cutoffCos) continue;
                    coneAccum += (cosA - scene.cone.cutoffCos)
                               / std::max(1e-4, 1.0 - scene.cone.cutoffCos)
                               * (1.0 - dist / scene.cone.range);
                }
                coneAccum *= 0.06 * scene.cone.intensity;
                if(coneAccum > 0.001){
                    auto addC = [](uint32_t a, double w, int shift){
                        const uint32_t v = static_cast<uint32_t>(
                            ((a >> shift) & 0xFF) * w + 0.5);
                        return v << shift;
                    };
                    const uint32_t warmR = addC(0x00FFEEDDull >> 0, 1.0, 16);
                    (void)warmR;
                    auto mixIn = [&](uint32_t baseC){
                        auto ch2 = [&](int shift){
                            const double b = (baseC >> shift) & 0xFF;
                            return static_cast<uint32_t>(b + (255 - b) * coneAccum * 0.5 + 0.5) << shift;
                        };
                        return 0xFF000000u | ch2(16) | ch2(8) | ch2(0);
                    };
                    color = mixIn(color);
                }
            }
            m_fb.setPixelOverlay(px, py, color);
        }
    }
}

}
