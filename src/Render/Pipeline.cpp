#include "Pipeline.hpp"
#include <cmath>

namespace Pipeline{

// End-to-end convention (lookAt + perspective + y-flip viewport):
// outward-facing triangles have NEGATIVE screen-space signed area.
bool isBackFacing(const ScreenVertex &a, const ScreenVertex &b, const ScreenVertex &c){
    double area = (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
    return area > 0;
}

namespace{
double NearDistance(const ScreenVertex &v){
    return v.z + v.w;   // clip-space near plane: z + w = 0
}

ScreenVertex LerpClip(const ScreenVertex &a, const ScreenVertex &b, double t){
    ScreenVertex r{};
    r.x = a.x + (b.x - a.x) * t;
    r.y = a.y + (b.y - a.y) * t;
    r.z = static_cast<float>(a.z + (b.z - a.z) * t);
    r.w = static_cast<float>(a.w + (b.w - a.w) * t);
    r.color = a.color;
    return r;
}
}

std::vector<ScreenTriangle> clipNearPlane(const ScreenVertex (&tri)[3]){
    int inCount = 0;
    for(int i = 0; i < 3; i++){
        if(NearDistance(tri[i]) >= 0) inCount++;
    }

    if(inCount == 3){
        return { ScreenTriangle{tri[0], tri[1], tri[2]} };
    }
    if(inCount == 0){
        return {};
    }

    // Sutherland-Hodgman against the single plane z+w=0
    std::vector<ScreenVertex> poly{};
    for(int i = 0; i < 3; i++){
        const ScreenVertex &cur = tri[i];
        const ScreenVertex &nxt = tri[(i+1)%3];
        double dCur = NearDistance(cur);
        double dNxt = NearDistance(nxt);
        bool curIn = dCur >= 0;
        bool nxtIn = dNxt >= 0;
        if(curIn) poly.push_back(cur);
        if(curIn != nxtIn){
            double t = dCur / (dCur - dNxt);
            poly.push_back(LerpClip(cur, nxt, t));
        }
    }

    std::vector<ScreenTriangle> res{};
    for(std::size_t i = 1; i + 1 < poly.size(); i++){
        res.push_back(ScreenTriangle{poly[0], poly[i], poly[i+1]});
    }
    return res;
}

std::vector<ScreenTriangle> projectObject(const Object4D &obj,
                                          const Matrix4DBase<double> &mvp,
                                          std::size_t screenW, std::size_t screenH){
    std::vector<ScreenTriangle> result{};
    result.reserve(obj.numPolys * 2);
    for(int p = 0; p < obj.numPolys; p++){
        const PolyF4D &poly = obj.plist[p];

        ScreenVertex sv[3]{};
        bool skip = false;
        for(int i = 0; i < 3; i++){
            auto clip = mvp.mul(poly.vlist[i]);   // w=1 points expected
            sv[i].x = clip.x;
            sv[i].y = clip.y;
            sv[i].z = static_cast<float>(clip.z);
            sv[i].w = static_cast<float>(clip.w);
            sv[i].color = poly.color;
            sv[i].u = static_cast<float>(poly.uvlist[i].u);
            sv[i].v = static_cast<float>(poly.uvlist[i].v);
            if(clip.w < 1e-6 && clip.z >= -clip.w){   // behind eye without valid w
                skip = true;
            }
        }
        if(skip) continue;

        auto clipped = clipNearPlane(sv);
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
