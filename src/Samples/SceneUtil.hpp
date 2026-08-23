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

}
