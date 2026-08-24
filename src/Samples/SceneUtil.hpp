#pragma once

#include "../Application.hpp"
#include "Render/TileRenderer.hpp"
#include "Render/Pipeline.hpp"
#include "Render/Rasterizer.hpp"

#include <cmath>
#include "imgui.h"

namespace SGE::Samples {

inline LightingRig makeDefaultRig(){
    LightingRig rig{};
    rig.ambient = 0.15f;
    DirectionalLight key{};
    key.direction = Vector3DBase<double>{-0.5, 0.8, -1.0};
    key.color = ColorFlt{1.0f, 1.0f, 1.0f};
    rig.directional.push_back(key);
    PointLight warm{};
    warm.position = Vector3DBase<double>{2.5, 2.5, -4.0};
    warm.color = ColorFlt{1.0f, 0.95f, 0.85f};
    warm.range = 12.0;
    rig.point.push_back(warm);
    return rig;
}

inline bool projectWorldPoint(const Matrix4DBase<double> &vp,
    const Point4D &p, int w, int h, ScreenVertex &out){
    const auto &M = vp;
    const double cx = M[0][0][0][0]*p.x + M[0][0][0][1]*p.y
        + M[0][0][0][2]*p.z + M[0][0][0][3]*p.w;
    const double cy = M[0][0][1][0]*p.x + M[0][0][1][1]*p.y
        + M[0][0][1][2]*p.z + M[0][0][1][3]*p.w;
    const double cz = M[0][0][2][0]*p.x + M[0][0][2][1]*p.y
        + M[0][0][2][2]*p.z + M[0][0][2][3]*p.w;
    const double cw = M[0][0][3][0]*p.x + M[0][0][3][1]*p.y
        + M[0][0][3][2]*p.z + M[0][0][3][3]*p.w;
    if(cw <= 1e-9){ return false; }
    out.x = static_cast<int>((cx / cw * 0.5 + 0.5) * w);
    out.y = static_cast<int>((-(cy / cw * 0.5) + 0.5) * h);
    out.z = static_cast<float>(cz / cw * 0.5 + 0.5);
    out.w = 1;
    return true;
}

inline FogParams defaultFog(Application &app){
    FogParams fog{};
    fog.start = app.fogStart();
    fog.end = app.fogEnd();
    fog.color = ColorFlt{0.45f, 0.55f, 0.70f};
    return fog;
}

inline Matrix4DBase<double> defaultViewProj(Application &app){
    auto view = app.camera().viewMatrix();
    auto proj = SGE::Math::perspective(M_PI / 3, 800.0 / 600.0, 0.1, 100.0);
    return proj.mul(view);
}

inline Object4D makePlane(double halfSize, double y,
    const Vector3DBase<double> &normal, const Color32 &color,
    double uvRepeat = 1.0){
    Object4D plane{};
    std::snprintf(plane.name, sizeof(plane.name), "%s", "plane");
    Point4D pv[4];
    if(std::fabs(normal.y) > 0.5){
        pv[0] = {-halfSize, y, -halfSize, 1};
        pv[1] = { halfSize, y, -halfSize, 1};
        pv[2] = { halfSize, y,  halfSize, 1};
        pv[3] = {-halfSize, y,  halfSize, 1};
    }else{
        pv[0] = {-halfSize, y - halfSize, 0, 1};
        pv[1] = { halfSize, y - halfSize, 0, 1};
        pv[2] = { halfSize, y + halfSize, 0, 1};
        pv[3] = {-halfSize, y + halfSize, 0, 1};
    }
    for(int i = 0; i < 4; i++){ plane.vlistLocal[i] = pv[i]; }
    plane.numVertices = 4;
    plane.numPolys = 2;
    const int idx[2][3] = {{0,1,2},{0,2,3}};
    const UV2D uvq[4] = {{0,0},{uvRepeat,0},{uvRepeat,uvRepeat},{0,uvRepeat}};
    for(int k = 0; k < 2; k++){
        for(int m = 0; m < 3; m++){
            plane.plist[k].vlist[m] = pv[idx[k][m]];
            plane.plist[k].uvlist[m] = uvq[idx[k][m]];
            plane.plist[k].nlist[m] = normal;
        }
        plane.plist[k].color = color;
    }
    return plane;
}

// ---- GraphicsAPILearn reference alignment helpers ----
inline constexpr uint32_t kRefClear = 0xFF1A1A1Au;  // (0.1,0.1,0.1)

inline SGE::Render::Camera refCamera(double x = 0, double y = 0, double z = 3,
                                     double yaw = 0, double pitch = 0){
    SGE::Render::Camera cam{};
    cam.position = Vector3DBase<double>{x, y, z};
    cam.yaw = yaw;
    cam.pitch = pitch;
    return cam;
}

// Reset the global (user-controlled) camera to a reference pose on scene
// switch, mirroring how each GL sample initializes its own camera.
inline void resetCamera(Application &app, double x = 0, double y = 0,
                        double z = 3, double yaw = 0, double pitch = 0){
    app.camera().position = Vector3DBase<double>{x, y, z};
    app.camera().yaw = yaw;
    app.camera().pitch = pitch;
}

inline Matrix4DBase<double> refViewProj(const SGE::Render::Camera &cam){
    auto view = cam.viewMatrix();
    auto proj = SGE::Math::perspective(M_PI / 3, 800.0 / 600.0, 0.1, 100.0);
    return proj.mul(view);
}

// unit cube (edge = 1) copied from app cube with vertices scaled by 0.5
inline Object4D unitCube(Application &app){
    Object4D c = app.cube();
    for(int i = 0; i < static_cast<int>(c.numVertices); i++){
        c.vlistLocal[static_cast<std::size_t>(i)].x *= 0.5;
        c.vlistLocal[static_cast<std::size_t>(i)].y *= 0.5;
        c.vlistLocal[static_cast<std::size_t>(i)].z *= 0.5;
    }
    for(int i = 0; i < static_cast<int>(c.numPolys); i++){
        for(int k = 0; k < 3; k++){
            c.plist[i].vlist[k].x *= 0.5;
            c.plist[i].vlist[k].y *= 0.5;
            c.plist[i].vlist[k].z *= 0.5;
        }
    }
    return c;
}

// reference ground plane: XZ [-5,5], y=-0.5, UV tiled 0..5
inline Object4D refPlane(const Color32 &color = Color32{255,255,255,255},
                         double uvTile = 5.0){
    Object4D plane{};
    std::snprintf(plane.name, sizeof(plane.name), "%s", "plane");
    Point4D pv[4] = {{-5,-0.5,-5,1},{5,-0.5,-5,1},{5,-0.5,5,1},{-5,-0.5,5,1}};
    for(int i = 0; i < 4; i++){ plane.vlistLocal[i] = pv[i]; }
    plane.numVertices = 4;
    plane.numPolys = 2;
    const int idx[2][3] = {{1,2,3},{1,3,0}};
    const UV2D uvq[4] = {{0,0},{uvTile,0},{uvTile,uvTile},{0,uvTile}};
    for(int k = 0; k < 2; k++){
        for(int m = 0; m < 3; m++){
            plane.plist[k].vlist[m] = pv[idx[k][m]];
            plane.plist[k].uvlist[m] = uvq[idx[k][m]];
            plane.plist[k].nlist[m] = Vector3DBase<double>{0, 1, 0};
        }
        plane.plist[k].color = color;
    }
    return plane;
}

// lamp bulb marker: white sphere scaled 0.2 at position
inline void drawLamp(Application &app, Rasterizer &rz,
                     const Vector3DBase<double> &pos,
                     double scale = 0.2,
                     const Color32 &color = Color32{255,255,255,255},
                     bool emissiveWhite = true){
    static Object4D proto = SGE::Render::MakeSphere(1.0, 14, 9);
    Object4D lamp = proto;
    for(int li = 0; li < static_cast<int>(lamp.numVertices); li++){
        lamp.vlistLocal[static_cast<std::size_t>(li)].x *= scale;
        lamp.vlistLocal[static_cast<std::size_t>(li)].y *= scale;
        lamp.vlistLocal[static_cast<std::size_t>(li)].z *= scale;
    }
    for(int pi2 = 0; pi2 < static_cast<int>(lamp.numPolys); pi2++){
        for(int k = 0; k < 3; k++){
            lamp.plist[static_cast<std::size_t>(pi2)].vlist[k].x *= scale;
            lamp.plist[static_cast<std::size_t>(pi2)].vlist[k].y *= scale;
            lamp.plist[static_cast<std::size_t>(pi2)].vlist[k].z *= scale;
        }
    }
    auto lm = SGE::Math::translation(pos.x, pos.y, pos.z);
    auto lnrm = SGE::Math::normalMatrix(lm);
    auto lt = Pipeline::projectObject(lamp, lm,
        defaultViewProj(app), lnrm, 800, 600);
    for(auto &t : lt){
        if(emissiveWhite){
            t.v[0].color = color; t.v[1].color = color; t.v[2].color = color;
            rz.drawTriangleSolid(t.v[0], t.v[1], t.v[2]);
        }else{
            rz.drawTriangleSolid(t.v[0], t.v[1], t.v[2]);
        }
    }
}

// Blinn-Phong point light shading params from the reference shaders
struct RefLight{
    Vector3DBase<double> pos{};
    float ambient{0.2f};
    float diffuse{1.0f};
    float specular{0.5f};
    float shininess{32.0f};
};

}
