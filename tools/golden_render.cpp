#include <cstdio>
#include <string>
#include <vector>
#include "Render/FrameBuffer.hpp"
#include "Render/Rasterizer.hpp"
#include "Render/Pipeline.hpp"
#include "Render/Texture.hpp"
#include "Render/Camera.hpp"
#include "Transform.hpp"
#include "Render/Primitives.hpp"
#include "Render/TileRenderer.hpp"
#include "Render/Shadow.hpp"
#include "Render/PostProcess.hpp"

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
    // keep in sync with Application::MakeCube (outward-facing winding)
    for(int i = 0;i < 12;i++){
        cube.plist[i].vlist[0] = v[faces[i].a];
        cube.plist[i].vlist[1] = v[faces[i].c];
        cube.plist[i].vlist[2] = v[faces[i].b];
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

namespace{

FrameBuffer g_fb{800, 600};

void WritePPM(const std::string &path){
    FILE *out = std::fopen(path.c_str(), "wb");
    if(!out){
        std::fprintf(stderr, "cannot open %s\n", path.c_str());
        std::exit(2);
    }
    std::fprintf(out, "P6\n800 600\n255\n");
    const auto *px = g_fb.colorData();
    for(std::size_t i = 0; i < 800u * 600u; i++){
        const unsigned char bgr[3] = {
            static_cast<unsigned char>(px[i] & 0xFF),
            static_cast<unsigned char>((px[i] >> 8) & 0xFF),
            static_cast<unsigned char>((px[i] >> 16) & 0xFF)};
        std::fwrite(bgr, 1, 3, out);
    }
    std::fclose(out);
}

// mode: shadow — spot shadow + PCF on ground/cube/cone/sphere (angle fixed)
void RenderShadow(){
    using namespace SGE::Render;
    g_fb.clear(0xFF000000u);
    Rasterizer rz{g_fb};
    LightingRig rig{};
    rig.ambient = 0.35f;
    Camera camera{};
    camera.position = Vector3DBase<double>{0, 0, -6};
    auto viewProj = SGE::Math::perspective(
        M_PI / 3, 800.0 / 600.0, 0.1, 100.0)
        .mul(camera.viewMatrix());
    const auto nrm = SGE::Math::normalMatrix(
        SGE::Math::translation(0.0, 0.0, 0.0));
    static FrameBuffer shadowMap{256, 256};
    shadowMap.clear();
    static Object4D ground{};
    Point4D gv[4] = {{-5,-1,-5,1},{5,-1,-5,1},{5,-1,5,1},{-5,-1,5,1}};
    for(int i = 0; i < 4; i++){ ground.vlistLocal[i] = gv[i]; }
    ground.numVertices = 4;
    ground.numPolys = 2;
    const int gi[2][3] = {{0,1,2},{0,2,3}};
    for(int k = 0; k < 2; k++){
        for(int m = 0; m < 3; m++){
            ground.plist[k].vlist[m] = gv[gi[k][m]];
            ground.plist[k].nlist[m] =
                Vector3DBase<double>{0, 1, 0};
        }
        ground.plist[k].color = Color32{210, 210, 220, 255};
    }
    std::fprintf(stderr, "shadow: prims done\n");
    static Object4D cone = SGE::Render::MakeCone(0.8, 2.0);
    std::fprintf(stderr, "shadow: cone ok\n");
    static Object4D sphere = SGE::Render::MakeSphere(1.0, 24, 16);
    const Vector3DBase<double> lightPos{4.0, 5.0, 3.0};
    const auto lightVP = pointLightVP(lightPos,
        Vector3DBase<double>{0, -1, 0}, M_PI / 2, 1.0, 0.5, 60.0);
    SpotLight spot{};
    spot.position = lightPos;
    spot.direction = Vector3DBase<double>{-4.0, -5.0, -3.0};
    spot.color = ColorFlt{1.0f, 0.97f, 0.9f};
    spot.range = 40.0;
    spot.cutoffCos = 0.55;
    rig.spot.push_back(spot);
    struct Ob{ const Object4D *obj; double x,y,z,ry; };
    const Ob obs[2] = {
        {&cone, -2.0, -0.5, 1.0, 0.0},
        {&sphere, 2.0, 0.2, -0.5, 0.0}};
    std::fprintf(stderr, "shadow: depth pass\n");
    {
        Rasterizer srz{shadowMap};
        auto gt = Pipeline::projectObject(ground,
            SGE::Math::translation(0.0,0.0,0.0),
            lightVP, nrm, 256, 256);
        for(auto &t : gt) srz.drawTriangleDepth(t.v[0], t.v[1], t.v[2]);
        for(const auto &ob : obs){
            auto om = SGE::Math::translation(ob.x, ob.y, ob.z);
            auto on = SGE::Math::normalMatrix(om);
            auto ot = Pipeline::projectObject(*ob.obj, om,
                lightVP, on, 256, 256);
            for(auto &t : ot)
                srz.drawTriangleDepth(t.v[0], t.v[1], t.v[2]);
        }
    }
    std::fprintf(stderr, "shadow: main pass\n");
    ShadowData sd{&shadowMap, lightVP, 0.004};
    sd.pcfRadius = 2;
    ShadingContext ctx{&rig, camera.position, nullptr, &sd};
    TileRenderer tiled{g_fb};
    for(const auto &ob : obs){
        auto om = SGE::Math::translation(ob.x, ob.y, ob.z);
        auto on = SGE::Math::normalMatrix(om);
        auto ot = Pipeline::projectObject(*ob.obj, om,
            viewProj, on, 800, 600);
        for(auto &t : ot) rz.drawTriangleSolid(t.v[0], t.v[1], t.v[2]);
    }
    auto gt2 = Pipeline::projectObject(ground,
        SGE::Math::translation(0.0,0.0,0.0), viewProj, nrm, 800, 600);
    tiled.drawTextured(gt2, MakeCheckerTexture(), &ctx);
}

// mode: pbr — small metal/roughness sphere row
void RenderPbr(){
    using namespace SGE::Render;
    g_fb.clear(0xFF000000u);
    Rasterizer rz{g_fb};
    LightingRig rig{};
    rig.ambient = 0.03f;
    for(int i = 0; i < 2; i++){
        PointLight p{};
        p.position = Vector3DBase<double>{i == 0 ? -8.0 : 8.0,
            8.0, 8.0};
        p.range = 200.0;
        rig.point.push_back(p);
    }
    Texture white(1, 1, std::vector<uint32_t>{0xFFFFFFFFu}.data());
    static Object4D proto = MakeSphere(0.45, 24, 16);
    Camera camera{};
    camera.position = Vector3DBase<double>{0, 0, -5};
    auto viewProj = SGE::Math::perspective(
        M_PI / 3, 800.0 / 600.0, 0.1, 100.0)
        .mul(camera.viewMatrix());
    for(int c = 0; c < 5; c++){
        PbrMaterial mat{};
        mat.baseColor = Color32{230, 80, 60, 255};
        mat.metallic = c / 4.0f;
        mat.roughness = 0.15f + 0.18f * c;
        ShadingContext ctx{&rig, camera.position,
            nullptr, nullptr, nullptr, nullptr, &mat};
        auto bm = SGE::Math::translation(-2.4 + c * 1.2, 0.0, 0.0);
        auto bnrm = SGE::Math::normalMatrix(bm);
        auto bt = Pipeline::projectObject(proto, bm,
            viewProj, bnrm, 800, 600);
        for(auto &t : bt){
            rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                    white, &ctx);
        }
    }
}

// mode: bloom — emissive quads + threshold blur additive
void RenderBloom(){
    using namespace SGE::Render;
    g_fb.clear(0xFF0A0A12u);
    Rasterizer rz{g_fb};
    ScreenVertex q[4] = {};
    q[0] = {300.0, 400.0, 0.5f, 1}; q[1] = {500.0, 400.0, 0.5f, 1};
    q[2] = {500.0, 200.0, 0.5f, 1}; q[3] = {300.0, 200.0, 0.5f, 1};
    for(int i = 0; i < 4; i++) q[i].color = Color32{255, 240, 200, 255};
    rz.drawTriangleSolid(q[0], q[1], q[2]);
    rz.drawTriangleSolid(q[0], q[2], q[3]);
    FrameBuffer bright{800, 600};
    ExtractBright(g_fb, bright, 0.7f);
    GaussianBlur(bright, 6);
    AdditiveBlend(g_fb, bright);
}

}


// mode: normal_map — brickwall plane with tangent-space normal perturbation
void RenderNormalMap(){
    using namespace SGE::Render;
    g_fb.clear(0xFF1A1A1Au);
    static Texture diffuse = ImageLoader::loadTexture(
        "assets/textures/brickwall.jpg");
    static Texture normalTex = ImageLoader::loadTexture(
        "assets/textures/brickwall_normal.jpg");
    LightingRig rig{};
    rig.ambient = 0.1f;
    rig.specularStrength = 0.2f;
    rig.shininess = 32.0f;
    PointLight p{};
    p.position = Vector3DBase<double>{0.0, 0.0, 1.0};
    p.range = 30.0;
    rig.point.push_back(p);

    Rasterizer rz{g_fb};
    const auto viewProj = SGE::Math::perspective(
        M_PI / 3, 800.0 / 600.0, 0.1, 100.0)
        .mul(SGE::Render::Camera{}.viewMatrix());
    ShadingContext ctx{&rig, SGE::Render::Camera{}.position};
    ctx.normalTex = &normalTex;
    ctx.tangentU = Vector3DBase<double>{1, 0, 0};
    ctx.tangentV = Vector3DBase<double>{0, 1, 0};
    Object4D wall{};
    Point4D pv[4] = {{-2.6,-1.2,2.2,1},{2.6,-1.2,2.2,1},
                     {2.6,1.4,2.2,1},{-2.6,1.4,2.2,1}};
    for(int i = 0; i < 4; i++){ wall.vlistLocal[i] = pv[i]; }
    wall.numVertices = 4;
    wall.numPolys = 2;
    const int wi[2][3] = {{0,1,2},{0,2,3}};
    const UV2D wuv[4] = {{0,0},{4,0},{4,4},{0,4}};
    for(int k = 0; k < 2; k++){
        for(int m = 0; m < 3; m++){
            wall.plist[k].vlist[m] = pv[wi[k][m]];
            wall.plist[k].uvlist[m] = wuv[wi[k][m]];
            wall.plist[k].nlist[m] =
                Vector3DBase<double>{0, 0, -1};
        }
        wall.plist[k].color = Color32{255,255,255,255};
    }
    auto wm = SGE::Math::translation(0.0, 0.0, 0.0);
    auto wnrm = SGE::Math::normalMatrix(wm);
    TileRenderer tiled{g_fb};
    auto wt = Pipeline::projectObject(wall, wm,
        viewProj, wnrm, 800, 600);
    tiled.drawTextured(wt, diffuse, &ctx);
}

int main(int argc, char **argv){
    if(argc < 2){
        std::fprintf(stderr, "usage: golden_render <out.ppm> [mode]\n"
                             "modes: lit_cube (default), shadow, pbr, "
                             "bloom\n");
        return 2;
    }
    const std::string mode = argc >= 3 ? argv[2] : "lit_cube";

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

    FrameBuffer &fb = g_fb;
    fb.clear(0xFF000000u);
    Rasterizer rz{fb};
    for(auto &t : ::Pipeline::projectObject(cube, model, viewProj, nrm, 800, 600)){
        rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2], checker, &shading);
    }

    if(mode == "shadow"){
        RenderShadow();
        WritePPM(argv[1]);
        return 0;
    }
    if(mode == "pbr"){
        RenderPbr();
        WritePPM(argv[1]);
        return 0;
    }
    if(mode == "normal_map"){
        RenderNormalMap();
        WritePPM(argv[1]);
        return 0;
    }
    if(mode == "bloom"){
        RenderBloom();
        WritePPM(argv[1]);
        return 0;
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
