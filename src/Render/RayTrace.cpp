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

    const uint32_t local = shade(rig, hit.albedo, hit.normal, hit.point, viewPos, shadowFactor);

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
            m_fb.setPixel(px, py, color, -1.0f);
        }
    }
}

}
