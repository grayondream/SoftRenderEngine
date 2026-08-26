#include <chrono>
#include <cstdio>
#include <vector>
#include "Render/FrameBuffer.hpp"
#include "Render/Rasterizer.hpp"
#include "Render/TileRenderer.hpp"
#include "Render/Pipeline.hpp"
#include "Render/Texture.hpp"
#include "Render/Camera.hpp"
#include "Transform.hpp"

namespace{

Object4D MakeCube(){
    Object4D cube{};
    std::snprintf(cube.name, sizeof(cube.name), "%s", "cube");
    double s = 1.0;
    Point4D v[8] = {{-s,-s,-s,1},{s,-s,-s,1},{s,s,-s,1},{-s,s,-s,1},
                    {-s,-s, s,1},{s,-s, s,1},{s,s, s,1},{-s,s, s,1}};
    for(int i = 0;i < 8;i++){ cube.vlistLocal[i] = v[i]; }
    cube.numVertices = 8;
    struct Face{ int a,b,c; };
    const Face faces[12] = {
        {0,3,2},{0,2,1},{4,5,6},{4,6,7},
        {0,1,5},{0,5,4},{3,7,6},{3,6,2},
        {1,2,6},{1,6,5},{0,4,7},{0,7,3}};
    cube.numPolys = 12;
    for(int i = 0;i < 12;i++){
        // keep in sync with Application::MakeCube (outward-facing winding)
        cube.plist[i].vlist[0] = v[faces[i].a];
        cube.plist[i].vlist[1] = v[faces[i].c];
        cube.plist[i].vlist[2] = v[faces[i].b];
        cube.plist[i].color = Color32{255, 255, 255, 255};
    }
    const Vector3DBase<double> n[6] = {
        {0,0,-1},{0,0,1},{0,-1,0},{0,1,0},{1,0,0},{-1,0,0}};
    for(int i = 0;i < 12;i++){
        for(int k = 0;k < 3;k++){ cube.plist[i].nlist[k] = n[i / 2]; }
    }
    return cube;
}

Texture MakeCheckerTexture(){
    constexpr std::size_t kSide = 8;
    std::vector<uint32_t> px(kSide * kSide);
    for(std::size_t y = 0; y < kSide; y++){
        for(std::size_t x = 0; x < kSide; x++){
            px[y * kSide + x] = (((x >> 1) + (y >> 1)) % 2 == 0)
                              ? 0xFFFFFFFFu : 0xFF202020u;
        }
    }
    return Texture(kSide, kSide, px.data());
}

}

int main(){
    auto cube = MakeCube();
    auto tex = MakeCheckerTexture();

    LightingRig rig{};
    rig.ambient = 0.15f;
    DirectionalLight key{};
    key.direction = Vector3DBase<double>{-0.5, 0.8, -1.0};
    key.color = ColorFlt{1.0f, 1.0f, 1.0f};
    rig.directional.push_back(key);
    ShadingContext ctx{&rig, Vector3DBase<double>{0, 2, -6}};

    constexpr int kW = 1280, kH = 720;
    FrameBuffer fb{kW, kH};

    double totalSerialMs = 0, totalTiledMs = 0;
    constexpr int kFrames = 60;

    for(int f = 0; f <= kFrames; f++){
        const double angle = 0.02 * f;
        auto model = SGE::Math::translation(0.0, 0.0, 0.0)
            .mul(SGE::Math::rotationY(angle))
            .mul(SGE::Math::rotationX(0.4));
        auto view = SGE::Render::Camera{}.viewMatrix();
        auto proj = SGE::Math::perspective(M_PI/3, static_cast<double>(kW)/kH, 0.1, 100.0);
        auto vp = proj.mul(view);
        auto nrm = SGE::Math::normalMatrix(model);
        auto tris = Pipeline::projectObject(cube, model, vp, nrm, kW, kH);

        if(f == 0){
            fb.clear();
            Rasterizer rz{fb};
            for(auto &t : tris){ rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2], tex, &ctx); }
            continue;
        }

        auto t0 = std::chrono::high_resolution_clock::now();
        fb.clear();
        Rasterizer rz{fb};
        for(auto &t : tris){ rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2], tex, &ctx); }
        auto t1 = std::chrono::high_resolution_clock::now();
        totalSerialMs += std::chrono::duration<double, std::milli>(t1 - t0).count();

        t0 = std::chrono::high_resolution_clock::now();
        fb.clear();
        SGE::Render::TileRenderer tiled{fb};
        tiled.drawTextured(tris, tex, &ctx);
        t1 = std::chrono::high_resolution_clock::now();
        totalTiledMs += std::chrono::duration<double, std::milli>(t1 - t0).count();
    }

    std::printf("frames=%d res=%dx%d\n", kFrames, kW, kH);
    std::printf("serial: %8.3f ms total, %7.3f ms/frame\n", totalSerialMs, totalSerialMs / kFrames);
    std::printf("tiled : %8.3f ms total, %7.3f ms/frame\n", totalTiledMs, totalTiledMs / kFrames);
    std::printf("speedup: %.2fx\n", totalSerialMs / totalTiledMs);
    return 0;
}
