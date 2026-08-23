#include <cstdio>
#include <string>
#include <vector>
#include "Render/FrameBuffer.hpp"
#include "Render/Rasterizer.hpp"
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
        cube.plist[i].vlist[0] = v[faces[i].a];
        cube.plist[i].vlist[1] = v[faces[i].b];
        cube.plist[i].vlist[2] = v[faces[i].c];
        cube.plist[i].color = Color32{255, 255, 255, 255};
    }
    const Vector3DBase<double> faceNormals[6] = {
        {0, 0, -1}, {0, 0, 1}, {0, -1, 0}, {0, 1, 0}, {1, 0, 0}, {-1, 0, 0}};
    for(int i = 0;i < 12;i++){
        for(int k = 0;k < 3;k++){
            cube.plist[i].nlist[k] = faceNormals[i / 2];
        }
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

int main(int argc, char **argv){
    if(argc < 2){
        std::fprintf(stderr, "usage: golden_render <out.ppm>\n");
        return 2;
    }

    Object4D cube = MakeCube();
    Texture checker = MakeCheckerTexture();
    const double angle = 0.02 * 60;
    auto model = SGE::Math::translation(0.0, 0.0, 0.0)
        .mul(SGE::Math::rotationY(angle))
        .mul(SGE::Math::rotationX(0.4));
    SGE::Render::Camera camera{};
    camera.position = Vector3DBase<double>{0, 2, -6};
    camera.yaw = 0;
    camera.pitch = -0.3217505543966422;
    auto view = camera.viewMatrix();
    auto proj = SGE::Math::perspective(M_PI/3, 800.0/600.0, 0.1, 100.0);
    auto viewProj = proj.mul(view);
    auto nrm = SGE::Math::normalMatrix(model);

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
    ShadingContext shading{&rig, camera.position};

    FrameBuffer fb{800, 600};
    fb.clear(0xFF000000u);
    Rasterizer rz{fb};
    for(auto &t : ::Pipeline::projectObject(cube, model, viewProj, nrm, 800, 600)){
        rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2], checker, &shading);
    }

    FILE *out = std::fopen(argv[1], "wb");
    if(!out){
        std::fprintf(stderr, "cannot open %s\n", argv[1]);
        return 2;
    }
    std::fprintf(out, "P6\n800 600\n255\n");
    const auto *px = fb.colorData();
    for(std::size_t i = 0; i < 800u * 600u; i++){
        const unsigned char bgr[3] = {
            static_cast<unsigned char>(px[i] & 0xFF),
            static_cast<unsigned char>((px[i] >> 8) & 0xFF),
            static_cast<unsigned char>((px[i] >> 16) & 0xFF)};
        std::fwrite(bgr, 1, 3, out);
    }
    std::fclose(out);
    return 0;
}
