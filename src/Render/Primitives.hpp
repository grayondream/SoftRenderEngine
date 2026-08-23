#pragma once
#include "GeoObject/Object4D.hpp"
#include <cmath>
#include <unordered_map>
#include <vector>
#include "TeapotData.hpp"

namespace SGE::Render{

inline Object4D MakeEmptyObject(const char *name){
    Object4D o{};
    std::snprintf(o.name, sizeof(o.name), "%s", name);
    return o;
}

inline Vector3DBase<double> V3(const Point4D &p){
    return Vector3DBase<double>{p.x, p.y, p.z};
}

inline void AddIndexedTriangle(Object4D &o, int a, int b, int c,
                               const UV2D &ua, const UV2D &ub, const UV2D &uc,
                               const Vector3DBase<double> &na,
                               const Vector3DBase<double> &nb,
                               const Vector3DBase<double> &nc){
    const auto &pa = o.vlistLocal[a];
    const auto &pb = o.vlistLocal[b];
    const auto &pc = o.vlistLocal[c];
    const auto e1 = Vector3DBase<double>{pb.x-pa.x, pb.y-pa.y, pb.z-pa.z};
    const auto e2 = Vector3DBase<double>{pc.x-pa.x, pc.y-pa.y, pc.z-pa.z};
    if(e1.mul(e2).length() < 1e-12) return;
    PolyF4D &poly = o.plist[o.numPolys++];
    poly.color = Color32{255, 255, 255, 255};
    poly.vlist[0] = o.vlistLocal[a];
    poly.vlist[1] = o.vlistLocal[b];
    poly.vlist[2] = o.vlistLocal[c];
    poly.uvlist[0] = ua; poly.uvlist[1] = ub; poly.uvlist[2] = uc;
    poly.nlist[0] = na; poly.nlist[1] = nb; poly.nlist[2] = nc;
}

inline Object4D MakeSphere(double radius, int segU = 24, int segV = 16){
    Object4D o = MakeEmptyObject("sphere");
    for(int v = 0; v <= segV; v++){
        const double phi = M_PI * v / segV;
        for(int u = 0; u <= segU; u++){
            const double th = 2.0 * M_PI * u / segU;
            const double nx = std::sin(phi) * std::cos(th);
            const double ny = std::cos(phi);
            const double nz = std::sin(phi) * std::sin(th);
            o.vlistLocal[o.numVertices++] =
                Point4D{radius * nx, radius * ny, radius * nz, 1};
        }
    }
    for(int v = 0; v < segV; v++){
        const bool southPole = (v == 0);
        const bool northPole = (v == segV - 1);
        for(int u = 0; u < segU; u++){
            const int r0 = v * (segU + 1), r1 = (v + 1) * (segU + 1);
            const double uu0 = static_cast<double>(u) / segU;
            const double uu1 = static_cast<double>(u + 1) / segU;
            const double vv0 = static_cast<double>(v) / segV;
            const double vv1 = static_cast<double>(v + 1) / segV;
            if(!southPole){
                AddIndexedTriangle(o, r0+u, r1+u, r0+u+1,
                    {uu0,vv0}, {uu0,vv1}, {uu1,vv0},
                    V3(o.vlistLocal[r0+u]), V3(o.vlistLocal[r1+u]), V3(o.vlistLocal[r0+u+1]));
            }
            if(!northPole){
                AddIndexedTriangle(o, r0+u+1, r1+u, r1+u+1,
                    {uu1,vv0}, {uu0,vv1}, {uu1,vv1},
                    V3(o.vlistLocal[r0+u+1]), V3(o.vlistLocal[r1+u]), V3(o.vlistLocal[r1+u+1]));
            }
        }
    }
    return o;
}

inline Object4D MakeTorus(double R, double r, int segU = 32, int segV = 16){
    Object4D o = MakeEmptyObject("torus");
    for(int i = 0; i <= segU; i++){
        const double th = 2.0 * M_PI * i / segU;
        const double ct = std::cos(th), st = std::sin(th);
        for(int j = 0; j <= segV; j++){
            const double ph = 2.0 * M_PI * j / segV;
            const double cp = std::cos(ph), sp = std::sin(ph);
            const double nx = cp * ct, ny = sp, nz = cp * st;
            o.vlistLocal[o.numVertices++] =
                Point4D{R * ct + r * nx, r * ny, R * st + r * nz, 1};
        }
    }
    for(int i = 0; i < segU; i++){
        for(int j = 0; j < segV; j++){
            const int a = i * (segV + 1) + j;
            const int b = a + segV + 1;
            const double u0 = 2.0 * i / segU, u1 = 2.0 * (i + 1) / segU;
            const double v0 = static_cast<double>(j) / segV;
            const double v1 = static_cast<double>(j + 1) / segV;
            AddIndexedTriangle(o, a, b, a+1,
                {u0,v0}, {u1,v0}, {u0,v1},
                V3(o.vlistLocal[a]), V3(o.vlistLocal[b]), V3(o.vlistLocal[a+1]));
            AddIndexedTriangle(o, a+1, b, b+1,
                {u0,v1}, {u1,v0}, {u1,v1},
                V3(o.vlistLocal[a+1]), V3(o.vlistLocal[b]), V3(o.vlistLocal[b+1]));
        }
    }
    return o;
}

inline Object4D MakeCylinder(double radius, double height, int seg = 24){
    Object4D o = MakeEmptyObject("cylinder");
    const double hy = height * 0.5;
    for(int i = 0; i <= seg; i++){
        const double th = 2.0 * M_PI * i / seg;
        const double nx = std::cos(th), nz = std::sin(th);
        o.vlistLocal[o.numVertices++] = Point4D{radius*nx, -hy, radius*nz, 1};
        o.vlistLocal[o.numVertices++] = Point4D{radius*nx,  hy, radius*nz, 1};
    }
    for(int i = 0; i < seg; i++){
        const int b0 = i*2, t0 = i*2+1, b1 = i*2+2, t1 = i*2+3;
        const double u0 = static_cast<double>(i) / seg;
        const double u1 = static_cast<double>(i + 1) / seg;
        AddIndexedTriangle(o, b0, b1, t0,
            {u0,0},{u1,0},{u0,1},
            V3(o.vlistLocal[b0]), V3(o.vlistLocal[b1]), V3(o.vlistLocal[t0]));
        AddIndexedTriangle(o, t0, b1, t1,
            {u0,1},{u1,0},{u1,1},
            V3(o.vlistLocal[t0]), V3(o.vlistLocal[b1]), V3(o.vlistLocal[t1]));
    }
    return o;
}

inline Object4D MakeCone(double radius, double height, int seg = 24){
    Object4D o = MakeEmptyObject("cone");
    const double hy = height * 0.5;
    o.vlistLocal[o.numVertices++] = Point4D{0, hy, 0, 1};
    for(int i = 0; i <= seg; i++){
        const double th = 2.0 * M_PI * i / seg;
        o.vlistLocal[o.numVertices++] =
            Point4D{radius * std::cos(th), -hy, radius * std::sin(th), 1};
    }
    for(int i = 0; i < seg; i++){
        const Vector3DBase<double> apex{0, hy, 0};
        const Vector3DBase<double> b0 = V3(o.vlistLocal[1+i]);
        const Vector3DBase<double> b1 = V3(o.vlistLocal[2+i]);
        const auto e1v = Vector3DBase<double>{b1.x-b0.x, b1.y-b0.y, b1.z-b0.z};
        const auto e2v = Vector3DBase<double>{apex.x-b0.x, apex.y-b0.y, apex.z-b0.z};
        auto n = e1v.mul(e2v);
        if(n.length() > 1e-12) n = n.normalize();
        const double u0 = static_cast<double>(i) / seg;
        const double u1 = static_cast<double>(i + 1) / seg;
        AddIndexedTriangle(o, 0, 1+i, 2+i,
            {0.5,1},{u0,0},{u1,0}, n, n, n);
    }
    return o;
}

inline Object4D MakeTeapot(int subdiv = 4){
    Object4D o = MakeEmptyObject("teapot");
    const auto &V = TeapotData::Vertices;
    const auto &P = TeapotData::Patches;
    const double kScale = 0.75;
    std::vector<Vector3DBase<double>> vnormals(
        static_cast<std::size_t>(32 * (subdiv+1) * (subdiv+1)));

    auto bern = [](int i, double t){
        const double s = 1.0 - t;
        switch(i){
            case 0: return s*s*s;
            case 1: return 3.0*t*s*s;
            case 2: return 3.0*t*t*s;
            default: return t*t*t;
        }
    };
    auto dbern = [](int i, double t){
        const double s = 1.0 - t;
        switch(i){
            case 0: return -3.0*s*s;
            case 1: return 9.0*t*s - 6.0*t*t;
            case 2: return 6.0*t - 9.0*t*t;
            default: return 3.0*t*t;
        }
    };

    for(const auto &patch : P){
        const int base = o.numVertices;
        for(int iv = 0; iv <= subdiv; iv++){
            const double v = static_cast<double>(iv) / subdiv;
            for(int iu = 0; iu <= subdiv; iu++){
                const double u = static_cast<double>(iu) / subdiv;
                double px = 0, py = 0, pz = 0;
                double sux = 0, suy = 0, suz = 0;
                double svx = 0, svy = 0, svz = 0;
                for(int j = 0; j < 4; j++){
                    for(int ii = 0; ii < 4; ii++){
                        const auto &cp = V[patch[j*4+ii] - 1];
                        const double bu = bern(ii,u), bv = bern(j,v);
                        const double du = dbern(ii,u), dv = dbern(j,v);
                        px += cp[0]*bu*bv;  py += cp[1]*bu*bv;  pz += cp[2]*bu*bv;
                        sux += cp[0]*du*bv; suy += cp[1]*du*bv; suz += cp[2]*du*bv;
                        svx += cp[0]*bu*dv; svy += cp[1]*bu*dv; svz += cp[2]*bu*dv;
                    }
                }
                Vector3DBase<double> n{suy*svz-suz*svy, suz*svx-sux*svz, sux*svy-suy*svx};
                if(n.length() > 1e-12) n = n.normalize();
                o.vlistLocal[base + iv*(subdiv+1) + iu] =
                    Point4D{kScale*px, kScale*pz, -kScale*py, 1};
                vnormals[static_cast<std::size_t>(base + iv*(subdiv+1) + iu)] =
                    Vector3DBase<double>{n.x, n.z, -n.y};
            }
        }
        for(int iv = 0; iv < subdiv; iv++){
            for(int iu = 0; iu < subdiv; iu++){
                const int r0 = base + iv*(subdiv+1), r1 = base + (iv+1)*(subdiv+1);
                const double u0 = static_cast<double>(iu)/subdiv;
                const double u1 = static_cast<double>(iu+1)/subdiv;
                const double v0 = static_cast<double>(iv)/subdiv;
                const double v1 = static_cast<double>(iv+1)/subdiv;
                AddIndexedTriangle(o, r0+iu, r1+iu, r0+iu+1,
                    {u0,v0},{u0,v1},{u1,v0},
                    vnormals[static_cast<std::size_t>(r0+iu)],
                    vnormals[static_cast<std::size_t>(r1+iu)],
                    vnormals[static_cast<std::size_t>(r0+iu+1)]);
                AddIndexedTriangle(o, r0+iu+1, r1+iu, r1+iu+1,
                    {u1,v0},{u0,v1},{u1,v1},
                    vnormals[static_cast<std::size_t>(r0+iu+1)],
                    vnormals[static_cast<std::size_t>(r1+iu)],
                    vnormals[static_cast<std::size_t>(r1+iu+1)]);
            }
        }
        o.numVertices += (subdiv + 1) * (subdiv + 1);
    }
    return o;
}


}