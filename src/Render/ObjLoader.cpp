#include "ObjLoader.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>

namespace{
constexpr std::size_t kMaxObjUVs = 4096;
constexpr std::size_t kMaxObjNormals = 4096;

struct ObjVertexRef{
    int v = 0;
    int vt = 0;
    int vn = 0;
};

bool ParseInt(const std::string &s, int &out){
    if(s.empty()) return false;
    std::istringstream ss(s);
    int v = 0;
    char c = '\0';
    if(!(ss >> v)) return false;
    if(ss >> c) return false;
    out = v;
    return true;
}

bool ParseVertexRef(const std::string &tok, ObjVertexRef &ref){
    ref = ObjVertexRef{};
    const std::size_t s1 = tok.find('/');
    if(s1 == std::string::npos){
        return ParseInt(tok, ref.v);
    }
    if(!ParseInt(tok.substr(0, s1), ref.v)) return false;
    const std::size_t s2 = tok.find('/', s1 + 1);
    if(s2 == std::string::npos){
        return ParseInt(tok.substr(s1 + 1), ref.vt);
    }
    if(s2 == s1 + 1){
        return ParseInt(tok.substr(s2 + 1), ref.vn);
    }
    return ParseInt(tok.substr(s1 + 1, s2 - s1 - 1), ref.vt)
        && ParseInt(tok.substr(s2 + 1), ref.vn);
}

bool ResolveIndex(int raw, std::size_t count, int &out){
    if(raw > 0){
        if(static_cast<std::size_t>(raw) > count) return false;
        out = raw - 1;
        return true;
    }
    if(raw < 0){
        const int idx = static_cast<int>(count) + raw;
        if(idx < 0 || idx >= static_cast<int>(count)) return false;
        out = idx;
        return true;
    }
    return false;
}
}

bool loadObjFromFile(const std::string &path, Object4D &out){
    out.numVertices = 0;
    out.numPolys = 0;

    std::ifstream file(path);
    if(!file.is_open()){
        return false;
    }

    auto fail = [&out](){
        out.numVertices = 0;
        out.numPolys = 0;
        return false;
    };

    std::vector<Point4D> verts{};
    std::vector<UV2D> uvs{};
    std::vector<Vector3DBase<double>> normals{};

    bool firstLine = true;
    std::string line;
    while(std::getline(file, line)){
        if(firstLine){
            firstLine = false;
            if(line.size() >= 3 &&
               static_cast<unsigned char>(line[0]) == 0xEF &&
               static_cast<unsigned char>(line[1]) == 0xBB &&
               static_cast<unsigned char>(line[2]) == 0xBF){
                line.erase(0, 3);
            }
        }
        if(!line.empty() && line.back() == '\r') line.pop_back();
        std::istringstream ss(line);
        std::string tag;
        if(!(ss >> tag)) continue;

        if(tag == "v"){
            double x = 0, y = 0, z = 0;
            if(!(ss >> x >> y >> z)) return fail();
            if(verts.size() >= kObject4vListLen) return fail();
            verts.push_back(Point4D{x, y, z, 1});
        }else if(tag == "vt"){
            double u = 0, v = 0;
            if(!(ss >> u >> v)) return fail();
            if(uvs.size() >= kMaxObjUVs) return fail();
            uvs.push_back(UV2D{u, v});
        }else if(tag == "vn"){
            double x = 0, y = 0, z = 0;
            if(!(ss >> x >> y >> z)) return fail();
            if(normals.size() >= kMaxObjNormals) return fail();
            normals.push_back(Vector3DBase<double>{x, y, z});
        }else if(tag == "f"){
            std::vector<ObjVertexRef> refs{};
            std::string tok;
            while(ss >> tok){
                ObjVertexRef ref{};
                if(!ParseVertexRef(tok, ref)) return fail();
                refs.push_back(ref);
            }
            if(refs.size() < 3) return fail();
            for(std::size_t i = 1; i + 1 < refs.size(); i++){
                if(out.numPolys >= kPolyListLen) return fail();
                PolyF4D &poly = out.plist[out.numPolys++];
                poly.color = Color32{255, 255, 255, 255};
                const ObjVertexRef tri[3] = {refs[0], refs[i], refs[i + 1]};
                bool hasN = true;
                Point4D p[3]{};
                for(int k = 0; k < 3; k++){
                    int vi = -1;
                    if(!ResolveIndex(tri[k].v, verts.size(), vi)) return fail();
                    p[k] = verts[vi];
                    poly.vlist[k] = p[k];

                    int ui = -1;
                    if(tri[k].vt != 0){
                        if(!ResolveIndex(tri[k].vt, uvs.size(), ui)) return fail();
                        poly.uvlist[k] = uvs[ui];
                    }else{
                        poly.uvlist[k] = UV2D{};
                    }

                    int ni = -1;
                    if(tri[k].vn != 0){
                        if(!ResolveIndex(tri[k].vn, normals.size(), ni)) return fail();
                        poly.nlist[k] = normals[ni];
                    }else{
                        poly.nlist[k] = Vector3DBase<double>{};
                        hasN = false;
                    }
                }
                if(!hasN){
                    const Vector3DBase<double> e1{p[1].x - p[0].x, p[1].y - p[0].y, p[1].z - p[0].z};
                    const Vector3DBase<double> e2{p[2].x - p[0].x, p[2].y - p[0].y, p[2].z - p[0].z};
                    auto n = e1.mul(e2);
                    const double len = n.length();
                    if(len > 1e-12){
                        n = n.normalize();
                        poly.nlist[0] = n; poly.nlist[1] = n; poly.nlist[2] = n;
                    }
                }
            }
        }
    }

    out.numVertices = static_cast<int>(verts.size());
    for(int i = 0; i < out.numVertices; i++){
        out.vlistLocal[i] = verts[i];
    }
    std::snprintf(out.name, sizeof(out.name), "%s", "obj");
    return true;
}

// ---- multi-material chunked loader ----
bool loadObjMultiMaterial(const std::string &path,
                          std::vector<Object4D> &chunks,
                          std::vector<int> &faceMaterial,
                          ObjMaterialInfo &materials){
    chunks.clear();
    faceMaterial.clear();
    materials.names.clear();
    materials.diffuseMap.clear();
    {
        const std::size_t slash = path.find_last_of('/');
        materials.mtlDir = slash == std::string::npos
            ? std::string(".") : path.substr(0, slash);
    }

    std::ifstream file(path);
    if(!file.is_open()){
        return false;
    }

    std::vector<Point4D> verts;
    std::vector<UV2D> uvs;
    std::vector<Vector3DBase<double>> normals;

    Object4D current{};
    current.numVertices = 0;
    current.numPolys = 0;
    int currentMtl = 0;
    bool any = false;

    auto flushChunk = [&](){
        if(current.numPolys == 0){ return; }
        current.numVertices = static_cast<int>(verts.size());
        if(current.numVertices > kObject4vListLen){
            current.numVertices = kObject4vListLen;
        }
        for(int i = 0; i < current.numVertices; i++){
            current.vlistLocal[i] =
                verts[static_cast<std::size_t>(i)];
        }
        std::snprintf(current.name, sizeof(current.name), "%s", "obj");
        chunks.push_back(current);
        current = Object4D{};
        any = true;
    };

    std::string line;
    while(std::getline(file, line)){
        if(!line.empty() && line.back() == '\r') line.pop_back();
        std::istringstream ss(line);
        std::string tag;
        if(!(ss >> tag)) continue;

        if(tag == "v"){
            double x = 0, y = 0, z = 0;
            if(!(ss >> x >> y >> z)) return false;
            verts.push_back(Point4D{x, y, z, 1});
        }else if(tag == "vt"){
            double u = 0, v = 0;
            if(!(ss >> u >> v)) return false;
            uvs.push_back(UV2D{u, v});
        }else if(tag == "vn"){
            double x = 0, y = 0, z = 0;
            if(!(ss >> x >> y >> z)) return false;
            normals.push_back(Vector3DBase<double>{x, y, z});
        }else if(tag == "mtllib"){
            std::string mtlName;
            ss >> mtlName;
            std::ifstream mtlFile(materials.mtlDir + "/" + mtlName);
            std::string cur;
            while(mtlFile.good()){
                std::string mline;
                std::getline(mtlFile, mline);
                if(!mline.empty() && mline.back() == '\r') mline.pop_back();
                std::istringstream mss(mline);
                std::string mt;
                if(!(mss >> mt)) continue;
                if(mt == "newmtl"){
                    mss >> cur;
                    materials.names.push_back(cur);
                    materials.diffuseMap.emplace_back();
                }else if(mt == "map_Kd" && !cur.empty()){
                    std::string mp;
                    mss >> mp;
                    materials.diffuseMap.back() = mp;
                }
            }
        }else if(tag == "usemtl"){
            std::string nm;
            ss >> nm;
            for(std::size_t i = 0; i < materials.names.size(); i++){
                if(materials.names[i] == nm){
                    currentMtl = static_cast<int>(i);
                }
            }
        }else if(tag == "f"){
            std::vector<ObjVertexRef> refs{};
            std::string tok;
            while(ss >> tok){
                ObjVertexRef ref{};
                if(!ParseVertexRef(tok, ref)) return false;
                refs.push_back(ref);
            }
            if(refs.size() < 3) continue;
            for(std::size_t i = 1; i + 1 < refs.size(); i++){
                if(current.numPolys >= 900){
                    flushChunk();
                }
                PolyF4D &poly =
                    current.plist[current.numPolys++];
                poly.color = Color32{255, 255, 255, 255};
                const ObjVertexRef tri[3] = {refs[0], refs[i], refs[i + 1]};
                Point4D p[3]{};
                bool hasN = true;
                for(int k = 0; k < 3; k++){
                    int vi2 = -1;
                    if(!ResolveIndex(tri[k].v, verts.size(), vi2)){
                        return false;
                    }
                    p[k] = verts[vi2];
                    poly.vlist[k] = p[k];
                    int ui = -1;
                    if(tri[k].vt != 0){
                        if(!ResolveIndex(tri[k].vt, uvs.size(), ui)){
                            return false;
                        }
                        poly.uvlist[k] = uvs[ui];
                    }
                    int ni = -1;
                    if(tri[k].vn != 0){
                        if(!ResolveIndex(tri[k].vn, normals.size(), ni)){
                            return false;
                        }
                        poly.nlist[k] = normals[ni];
                    }else{
                        hasN = false;
                    }
                }
                if(!hasN){
                    const Vector3DBase<double> e1{
                        p[1].x - p[0].x, p[1].y - p[0].y,
                        p[1].z - p[0].z};
                    const Vector3DBase<double> e2{
                        p[2].x - p[0].x, p[2].y - p[0].y,
                        p[2].z - p[0].z};
                    auto n = e1.mul(e2);
                    if(n.length() > 1e-12){
                        n = n.normalize();
                        poly.nlist[0] = n;
                        poly.nlist[1] = n;
                        poly.nlist[2] = n;
                    }
                }
                faceMaterial.push_back(currentMtl);
            }
        }
    }
    flushChunk();
    return any;
}
