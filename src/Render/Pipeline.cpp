#include "Pipeline.hpp"
#include <cmath>

namespace Pipeline{

// End-to-end convention (lookAt + perspective + y-flip viewport):
// outward-facing triangles have NEGATIVE screen-space signed area.
bool isBackFacing(const ScreenVertex &a, const ScreenVertex &b, const ScreenVertex &c){
    double area = (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
    return area < 0;
}

namespace{
ScreenVertex LerpClip(const ScreenVertex &a, const ScreenVertex &b, double t){
    ScreenVertex r{};
    r.x = a.x + (b.x - a.x) * t;
    r.y = a.y + (b.y - a.y) * t;
    r.z = static_cast<float>(a.z + (b.z - a.z) * t);
    r.w = static_cast<float>(a.w + (b.w - a.w) * t);
    r.u = static_cast<float>(a.u + (b.u - a.u) * t);
    r.v = static_cast<float>(a.v + (b.v - a.v) * t);
    r.nx = a.nx + (b.nx - a.nx) * t;
    r.ny = a.ny + (b.ny - a.ny) * t;
    r.nz = a.nz + (b.nz - a.nz) * t;
    r.wx = a.wx + (b.wx - a.wx) * t;
    r.wy = a.wy + (b.wy - a.wy) * t;
    r.wz = a.wz + (b.wz - a.wz) * t;
    r.color = a.color;
    return r;
}
}

namespace{
struct FrustumPlane{ double a, b, c, d; };

double PlaneDist(const FrustumPlane &pl, const ScreenVertex &v){
    return pl.a*v.x + pl.b*v.y + pl.c*v.z + pl.d*v.w;
}

std::vector<ScreenVertex> ClipPolygon(const std::vector<ScreenVertex> &poly,
                                      const FrustumPlane &pl){
    std::vector<ScreenVertex> out{};
    for(std::size_t i = 0; i < poly.size(); i++){
        const ScreenVertex &cur = poly[i];
        const ScreenVertex &nxt = poly[(i+1)%poly.size()];
        const double dCur = PlaneDist(pl, cur);
        const double dNxt = PlaneDist(pl, nxt);
        const bool curIn = dCur >= 0;
        const bool nxtIn = dNxt >= 0;
        if(curIn) out.push_back(cur);
        if(curIn != nxtIn){
            const double t = dCur / (dCur - dNxt);
            out.push_back(LerpClip(cur, nxt, t));
        }
    }
    return out;
}
}

std::vector<ScreenTriangle> clipTriangle(const ScreenVertex (&tri)[3]){
    static const FrustumPlane kPlanes[6] = {
        {0,0,1,1}, {0,0,-1,1}, {1,0,0,1}, {-1,0,0,1}, {0,1,0,1}, {0,-1,0,1}};
    std::vector<ScreenVertex> poly{tri[0], tri[1], tri[2]};
    for(const auto &pl : kPlanes){
        if(poly.empty()) break;
        poly = ClipPolygon(poly, pl);
    }

    std::vector<ScreenTriangle> res{};
    for(std::size_t i = 1; i + 1 < poly.size(); i++){
        res.push_back(ScreenTriangle{poly[0], poly[i], poly[i+1]});
    }
    return res;
}

std::vector<ScreenTriangle> projectObject(const Object4D &obj,
                                          const Matrix4DBase<double> &model,
                                          const Matrix4DBase<double> &viewProj,
                                          const Matrix3DBase<double> &normalMat,
                                          std::size_t screenW, std::size_t screenH,
                                          const Vector3DBase<double> *viewPos){
    std::vector<ScreenTriangle> result{};
    result.reserve(obj.numPolys * 2);
    for(int p = 0; p < obj.numPolys; p++){
        const PolyF4D &poly = obj.plist[p];

        ScreenVertex sv[3]{};
        bool skip = false;
        for(int i = 0; i < 3; i++){
            auto world = model.mul(poly.vlist[i]);
            auto clip = viewProj.mul(world);
            sv[i].x = clip.x;
            sv[i].y = clip.y;
            sv[i].z = static_cast<float>(clip.z);
            sv[i].w = static_cast<float>(clip.w);
            sv[i].u = static_cast<float>(poly.uvlist[i].u);
            sv[i].v = static_cast<float>(poly.uvlist[i].v);
            sv[i].color = poly.color;
            if(clip.w < 1e-6 && clip.z >= -clip.w){
                skip = true;
            }

            sv[i].wx = world.x;
            sv[i].wy = world.y;
            sv[i].wz = world.z;

            const auto &nl = poly.nlist[i];
            const double nxRaw = normalMat[0][0][0]*nl.x + normalMat[0][0][1]*nl.y + normalMat[0][0][2]*nl.z;
            const double nyRaw = normalMat[0][1][0]*nl.x + normalMat[0][1][1]*nl.y + normalMat[0][1][2]*nl.z;
            const double nzRaw = normalMat[0][2][0]*nl.x + normalMat[0][2][1]*nl.y + normalMat[0][2][2]*nl.z;
            const double len = std::sqrt(nxRaw*nxRaw + nyRaw*nyRaw + nzRaw*nzRaw);
            if(len > 1e-12){
                sv[i].nx = nxRaw / len;
                sv[i].ny = nyRaw / len;
                sv[i].nz = nzRaw / len;
            } else {
                sv[i].nx = 0; sv[i].ny = 0; sv[i].nz = 0;
            }
        }
        if(skip) continue;

        // fallback: zero normals (hand-built boxes without nlist) get a
        // geometric face normal so per-pixel lighting stays usable
        if(std::abs(sv[0].nx) + std::abs(sv[0].ny) + std::abs(sv[0].nz)
            + std::abs(sv[1].nx) + std::abs(sv[1].ny) + std::abs(sv[1].nz)
            + std::abs(sv[2].nx) + std::abs(sv[2].ny) + std::abs(sv[2].nz)
            < 1e-9){
            const double e1x = sv[1].wx - sv[0].wx,
                e1y = sv[1].wy - sv[0].wy, e1z = sv[1].wz - sv[0].wz;
            const double e2x = sv[2].wx - sv[0].wx,
                e2y = sv[2].wy - sv[0].wy, e2z = sv[2].wz - sv[0].wz;
            double fnx = e1y * e2z - e1z * e2y;
            double fny = e1z * e2x - e1x * e2z;
            double fnz = e1x * e2y - e1y * e2x;
            const double fl =
                std::sqrt(fnx*fnx + fny*fny + fnz*fnz);
            if(fl > 1e-12){
                // orient toward the viewer (two-sided lighting for boxes)
                const Vector3DBase<double> eye =
                    viewPos ? *viewPos
                        : Vector3DBase<double>{0, 0, 0};
                const double vx = eye.x - (sv[0].wx + sv[1].wx
                    + sv[2].wx) / 3.0;
                const double vy = eye.y - (sv[0].wy + sv[1].wy
                    + sv[2].wy) / 3.0;
                const double vz = eye.z - (sv[0].wz + sv[1].wz
                    + sv[2].wz) / 3.0;
                if(fnx*vx + fny*vy + fnz*vz < 0){
                    fnx = -fnx; fny = -fny; fnz = -fnz;
                }
                fnx /= fl; fny /= fl; fnz /= fl;
                for(int q = 0; q < 3; q++){
                    sv[q].nx = fnx; sv[q].ny = fny; sv[q].nz = fnz;
                }
            }
        }

        auto clipped = clipTriangle(sv);
        for(auto &t : clipped){
            bool ok = true;
            for(int i = 0; i < 3; i++){
                ScreenVertex &v = t.v[i];
                if(v.w <= 0){ ok = false; break; }
                double ndcX = v.x / v.w;
                double ndcY = v.y / v.w;
                double ndcZ = v.z / v.w;
                v.x = (ndcX * 0.5 + 0.5) * screenW;
                v.y = (1.0 - (ndcY * 0.5 + 0.5)) * screenH;
                v.z = static_cast<float>(ndcZ);
                // keep clip-space w: rasterizer needs it for perspective-correct depth
            }
            if(!ok) continue;
            if(isBackFacing(t.v[0], t.v[1], t.v[2])) continue;
            result.push_back(t);
        }
    }
    return result;
}

}
