#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <memory>
#include <ratio>
#include <system_error>
#include <thread>
#include <vector>
#include <SDL2/SDL.h>
#include "Application.hpp"
#include "BufferManager.hpp"
#include "Environment.hpp"
#include "ErrorCode.hpp"
#include "Window.hpp"
#include "Log.hpp"
#include "WindowBuffer.hpp"
#include "Render/TileRenderer.hpp"
#include "Render/Pipeline.hpp"
#include "Render/Texture.hpp"
#include "Render/Light.hpp"
#include "Render/RayTrace.hpp"
#include "Render/ObjLoader.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "Transform.hpp"

namespace{

std::sig_atomic_t g_stopRequested = 0;

extern "C" void handleStopSignal(int){
    g_stopRequested = 1;
}

Object4D MakeCube(){
    Object4D cube{};
    std::snprintf(cube.name, sizeof(cube.name), "%s", "cube");
    double s = 1.0;
    Point4D v[8] = {{-s,-s,-s,1},{s,-s,-s,1},{s,s,-s,1},{-s,s,-s,1},
                    {-s,-s, s,1},{s,-s, s,1},{s,s, s,1},{-s,s, s,1}};
    for(int i = 0;i < 8;i++){ cube.vlistLocal[i] = v[i]; }
    cube.numVertices = 8;

    struct Face{ int a,b,c; Color32 col; };
    const Face faces[12] = {
        {0,3,2, {0,255,0,255}}, {0,2,1, {0,255,0,255}},
        {4,5,6, {255,0,0,255}}, {4,6,7, {255,0,0,255}},
        {0,1,5, {0,0,255,255}}, {0,5,4, {0,0,255,255}},
        {3,7,6, {255,255,0,255}},{3,6,2, {255,255,0,255}},
        {1,2,6, {255,0,255,255}},{1,6,5, {255,0,255,255}},
        {0,4,7, {0,255,255,255}},{0,7,3, {0,255,255,255}},
    };
    cube.numPolys = 12;
    for(int i = 0;i < 12;i++){
        cube.plist[i].vlist[0] = v[faces[i].a];
        cube.plist[i].vlist[1] = v[faces[i].b];
        cube.plist[i].vlist[2] = v[faces[i].c];
        cube.plist[i].color = faces[i].col;
    }
    for(int i = 0;i < 12;i += 2){
        cube.plist[i].uvlist[0]   = {0, 0};
        cube.plist[i].uvlist[1]   = {1, 0};
        cube.plist[i].uvlist[2]   = {1, 1};
        cube.plist[i+1].uvlist[0] = {0, 0};
        cube.plist[i+1].uvlist[1] = {1, 1};
        cube.plist[i+1].uvlist[2] = {0, 1};
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

std::error_code Application::initalize(const ApplicationParam &param){
    if(auto err = Environment::instance()->initalize(param.env); err){
        return err;
    }

    m_pwindow = std::make_shared<Window>(param.env.pos, param.env.format);
    if(auto err = m_pwindow->init(); err){
        return err;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(m_pwindow->window(), m_pwindow->renderer());
    ImGui_ImplSDLRenderer2_Init(m_pwindow->renderer());

    m_cube = MakeCube();
    m_checker = MakeCheckerTexture();
    m_sphere = SGE::Render::MakeSphere(1.0, 24, 16);
    std::snprintf(m_sphere.name, sizeof(m_sphere.name), "%s", "sphere");
    m_sphere.worldPos = Point4D{-2.8, 0.2, 0, 1};
    m_torus = SGE::Render::MakeTorus(1.4, 0.45, 32, 16);
    m_torus.worldPos = Point4D{2.8, 0.0, 0.5, 1};
    m_teapot = SGE::Render::MakeTeapot();
    m_teapot.worldPos = Point4D{0, -1.55, 2.2, 1};
    m_rtScene.spheres.push_back(
        SGE::Render::RaySphere{Vector3DBase<double>{0, 0, 0}, 1.2,
                               Color32{255, 60, 60, 255}, 0.6f});
    m_rtScene.spheres.push_back(
        SGE::Render::RaySphere{Vector3DBase<double>{-2.2, -0.4, 0.5}, 0.8,
                               Color32{60, 120, 255, 255}, 0.1f});
    m_rtScene.triangles.push_back(
        SGE::Render::RayTriangle{Vector3DBase<double>{-6, -1.5, 4},
                                 Vector3DBase<double>{6, -1.5, 4},
                                 Vector3DBase<double>{0, -1.5, -4},
                                 Color32{180, 180, 180, 255}, 0.35f});
    return {};
}

 void Application::operator()(const WindowEventType t){
    switch(t){
        case WindowEventType::WINDOW_EVENT_QUIT:
            m_bQuit = true;
            break;
        default:
            //LOGI("Not handle {}", static_cast<int>(t));
            break;
    }
}

void Application::RenderScene(){
    const char *barsEnv = std::getenv("SGE_TEST_BARS");
    if(barsEnv){
        const uint32_t bars[4] = {
            0xFFFF0000u,
            0xFF00FF00u,
            0xFF0000FFu,
            0xFFFFFFFFu};
        for(std::size_t y = 0; y < 600; y++){
            for(std::size_t x = 0; x < 800; x++){
                m_framebuffer.setPixel(x, y, bars[x / 200], -3.0f);
            }
        }
        BufferManager::instance()->draw(
            reinterpret_cast<const uint8_t*>(m_framebuffer.colorData()));
        return;
    }
    if(m_rotating){
        m_angle += m_rotateSpeed;
    }
    auto model = SGE::Math::translation(m_cube.worldPos.x, m_cube.worldPos.y, m_cube.worldPos.z)
        .mul(SGE::Math::rotationY(m_angle))
        .mul(SGE::Math::rotationX(0.4));
    auto view = m_camera.viewMatrix();
    auto proj = SGE::Math::perspective(M_PI/3, 800.0/600.0, 0.1, 100.0);
    auto viewProj = proj.mul(view);
    auto nrm = SGE::Math::normalMatrix(model);

    m_rig = LightingRig{};
    m_rig.ambient = 0.15f;
    DirectionalLight key{};
    key.direction = Vector3DBase<double>{-0.5, 0.8, -1.0};
    key.color = ColorFlt{1.0f, 1.0f, 1.0f};
    m_rig.directional.push_back(key);
    PointLight warm{};
    warm.position = Vector3DBase<double>{2.5, 2.5, -4.0};
    warm.color = ColorFlt{1.0f, 0.95f, 0.85f};
    warm.range = 12.0;
    m_rig.point.push_back(warm);

    FogParams fog{};
    fog.start = m_fogStart;
    fog.end = m_fogEnd;
    fog.color = ColorFlt{0.45f, 0.55f, 0.70f};
    ShadingContext shading{&m_rig, m_camera.position, m_fogEnabled ? &fog : nullptr};

    m_framebuffer.clear(0xFF000000u);
    Rasterizer rz{m_framebuffer};

    switch(m_sceneMode){
        case 1:
            for(auto &t : Pipeline::projectObject(m_cube, model, viewProj, nrm, 800, 600)){
                rz.drawTriangleWireframe(t.v[0], t.v[1], t.v[2]);
            }
            break;
        case 2:
        {
            const auto rot = SGE::Math::rotationY(m_angle * 0.5);
            struct Item{ const Object4D *obj; double ox, oy, oz; };
            const Item items[] = {
                {&m_sphere, m_sphere.worldPos.x, m_sphere.worldPos.y + 0.8, m_sphere.worldPos.z},
                {&m_torus,  m_torus.worldPos.x,  m_torus.worldPos.y + 1.0,  m_torus.worldPos.z},
                {&m_teapot, m_teapot.worldPos.x, m_teapot.worldPos.y + 0.75, m_teapot.worldPos.z},
            };
            for(const auto &it : items){
                auto im = SGE::Math::translation(it.ox, it.oy, it.oz).mul(rot);
                auto inrm = SGE::Math::normalMatrix(im);
                auto t2 = Pipeline::projectObject(*it.obj, im, viewProj, inrm, 800, 600);
                for(auto &t : t2){
                    rz.drawTriangleSolid(t.v[0], t.v[1], t.v[2]);
                }
            }
            break;
        }
        case 3:
        {
            ShadingContext unlit{nullptr, m_camera.position};
            SGE::Render::TileRenderer tiled{m_framebuffer};
            auto tris = Pipeline::projectObject(m_cube, model, viewProj, nrm, 800, 600);
            tiled.drawTextured(tris, m_checker, &unlit);
            break;
        }
        case 4:
        {
            auto tris = Pipeline::projectObject(m_cube, model, viewProj, nrm, 800, 600);
            for(auto &t : tris){
                t.v[0].color = Color32{255, 60, 60, 160};
                t.v[1].color = Color32{255, 60, 60, 160};
                t.v[2].color = Color32{255, 60, 60, 160};
                rz.drawTriangleSolid(t.v[0], t.v[1], t.v[2]);
            }
            ScreenVertex ov[3] = {};
            ov[0] = {300, 500, -0.4f, 1};
            ov[1] = {700, 200, -0.4f, 1};
            ov[2] = {250, 150, -0.4f, 1};
            ov[0].color = Color32{40, 200, 90, 110};
            ov[1].color = Color32{40, 200, 90, 110};
            ov[2].color = Color32{40, 200, 90, 110};
            rz.drawTriangleSolid(ov[0], ov[1], ov[2]);
            break;
        }
        case 5:
        {
            Object4D obj{};
            if(loadObjFromFile("assets/cube.obj", obj)){
                auto tris = Pipeline::projectObject(obj, model, viewProj, nrm, 800, 600);
                for(auto &t : tris){
                    rz.drawTriangleSolid(t.v[0], t.v[1], t.v[2]);
                }
            }
            break;
        }
        default:
        {
            SGE::Render::TileRenderer tiled{m_framebuffer};
            auto tris = Pipeline::projectObject(m_cube, model, viewProj, nrm, 800, 600);
            tiled.drawTextured(tris, m_checker, &shading);
            break;
        }
    }

    if(m_sceneMode == 6){
        FrameBuffer shadowMap{256, 256};
        shadowMap.clear();
        Object4D ground{};
        std::snprintf(ground.name, sizeof(ground.name), "%s", "ground");
        const double e = 6.0;
        Point4D gv[4] = {{-e,-2,-e,1},{e,-2,-e,1},{e,-2,e,1},{-e,-2,e,1}};
        for(int i = 0;i < 4;i++){ ground.vlistLocal[i] = gv[i]; }
        ground.numVertices = 4;
        ground.numPolys = 2;
        const int idx[2][3] = {{0,1,2},{0,2,3}};
        for(int i = 0;i < 2;i++){
            for(int k = 0;k < 3;k++){
                ground.plist[i].vlist[k] = gv[idx[i][k]];
                ground.plist[i].nlist[k] = Vector3DBase<double>{0, 1, 0};
            }
            ground.plist[i].color = Color32{210, 210, 220, 255};
        }

        const auto lightVP = SGE::Render::directionalLightVP(
            Vector3DBase<double>{-0.5, 0.8, -1.0},
            Vector3DBase<double>{0, -0.5, 0}, 7.0);
        {
            Rasterizer srz{shadowMap};
            auto depthTris = Pipeline::projectObject(m_cube, model,
                lightVP, nrm, 256, 256);
            for(auto &t : depthTris){
                srz.drawTriangleDepth(t.v[0], t.v[1], t.v[2]);
            }
            auto gtris = Pipeline::projectObject(ground, SGE::Math::translation(0.0,0.0,0.0),
                lightVP, nrm, 256, 256);
            for(auto &t : gtris){
                srz.drawTriangleDepth(t.v[0], t.v[1], t.v[2]);
            }
        }

        SGE::Render::ShadowData sd{&shadowMap, lightVP, 0.008};
        ShadingContext shadCtx{&m_rig, m_camera.position,
                               m_fogEnabled ? &fog : nullptr, &sd};
        {
            SGE::Render::TileRenderer tiled{m_framebuffer};
            auto tris = Pipeline::projectObject(m_cube, model, viewProj, nrm, 800, 600);
            tiled.drawTextured(tris, m_checker, &shadCtx);
            auto gtris = Pipeline::projectObject(ground,
                SGE::Math::translation(0.0,0.0,0.0), viewProj, nrm, 800, 600);
            tiled.drawTextured(gtris, m_checker, &shadCtx);
        }
    }else if(m_sceneMode == 7){
        SGE::Render::RayTraceOptions opt{};
        opt.maxDepth = 3;
        opt.background = Color32{25, 28, 40, 255};
        m_rtBuffer.clear();
        SGE::Render::RayTracer tracer{m_rtBuffer};
        tracer.render(m_rtScene, m_camera, m_rig, opt);
        const auto *src = m_rtBuffer.colorData();
        const std::size_t rw = m_rtBuffer.width(), rh = m_rtBuffer.height();
        for(std::size_t y = 0; y < 600; y++){
            const std::size_t sy = y * rh / 600;
            for(std::size_t x = 0; x < 800; x++){
                const std::size_t sx = x * rw / 800;
                m_framebuffer.setPixel(x, y, src[sy * rw + sx], -2.0f);
            }
        }
    }

    BufferManager::instance()->draw(
        reinterpret_cast<const uint8_t*>(m_framebuffer.colorData()));
}

void Application::RenderDebugUi(){
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("SoftEngine Debug");
    const char *modes[] = {
        "0 Lit+Texture (tiled)", "1 Wireframe", "2 Flat Solid",
        "3 Unlit Texture", "4 Alpha Blend", "5 OBJ Mesh",
        "6 Shadow Ground", "7 Ray Traced"};
    if(ImGui::Combo("Scene", &m_sceneMode, modes, IM_ARRAYSIZE(modes))){
        LOGI("[UI] scene -> {} via ImGui", m_sceneMode);
        m_framebuffer.clearDepth();
    }
    ImGui::Text("Current scene: %d", m_sceneMode);
    ImGui::Checkbox("Rotating", &m_rotating);
    ImGui::SliderFloat("Speed", &m_rotateSpeed, 0.0f, 0.3f, "%.3f");
    ImGui::Separator();
    ImGui::Checkbox("Fog", &m_fogEnabled);
    if(m_fogEnabled){
        float startEnd[2] = {m_fogStart, m_fogEnd};
        if(ImGui::SliderFloat2("Fog Range", startEnd, 0.0f, 60.0f, "%.1f")){
            m_fogStart = std::min(startEnd[0], startEnd[1] - 0.5f);
            m_fogEnd = std::max(startEnd[1], startEnd[0] + 0.5f);
        }
    }
    ImGui::Separator();
    ImGui::Text("Camera: WASD move / RF up-down");
    ImGui::Text("pos=(%.1f, %.1f, %.1f)",
                m_camera.position.x, m_camera.position.y, m_camera.position.z);
    ImGui::End();

    ImGui::Render();
}

void Application::ShutdownUi(){
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

std::error_code Application::run(){
    if(!m_pwindow){
        LOGE("The window is nullptr, can not display any scene!");
        return std::error_code(static_cast<int>(ErrorCode::FAILED), std::generic_category());
    }

    m_pwindow->setListener(this);
    std::signal(SIGINT, handleStopSignal);
    std::signal(SIGTERM, handleStopSignal);
    const char *maxFramesEnv = std::getenv("SGE_MAX_FRAMES");
    const int64_t maxFrames = maxFramesEnv ? std::atoll(maxFramesEnv) : 0;
    const std::chrono::high_resolution_clock::time_point pt = std::chrono::high_resolution_clock::now();
    auto lastFrameTime = pt;
    int64_t delaTime = 1000;    //ms
    int64_t cycleCount = 0;
    BufferManager::instance()->clear(GenerateColor());
    while(!m_bQuit){
        if(g_stopRequested || (maxFrames > 0 && cycleCount >= maxFrames)){
            SDL_Event quit{};
            quit.type = SDL_QUIT;
            SDL_PushEvent(&quit);
        }
        cycleCount += 1;
        auto durationPerCycle = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - pt).count();
        if(durationPerCycle >= delaTime){
            auto fps = cycleCount * 1000.0/ durationPerCycle;
            m_pwindow->setTitle(std::format("Game Engine: Fps {:.2f}", fps));
        }

        m_pwindow->processEvent();

        auto frameNow = std::chrono::high_resolution_clock::now();
        const double frameDt = std::chrono::duration<double>(frameNow - lastFrameTime).count();
        lastFrameTime = frameNow;

        int kbCount = 0;
        const Uint8 *kb = SDL_GetKeyboardState(&kbCount);
        if(kb && kbCount > 0 && kb[SDL_SCANCODE_RIGHTBRACKET] && !m_bracketHeld){
            m_sceneMode = (m_sceneMode + 1) % 8;
            m_framebuffer.clearDepth();
            LOGI("[KEY] scene -> {} via ]", m_sceneMode);
        }
        if(kb && kbCount > 0 && kb[SDL_SCANCODE_LEFTBRACKET] && !m_bracketHeld){
            m_sceneMode = (m_sceneMode + 7) % 8;
            m_framebuffer.clearDepth();
            LOGI("[KEY] scene -> {} via [", m_sceneMode);
        }
        m_bracketHeld = kb && kbCount > 0 &&
            (kb[SDL_SCANCODE_RIGHTBRACKET] || kb[SDL_SCANCODE_LEFTBRACKET]);

        SGE::Render::InputState in{};
        if(kb && kbCount > 0){
            in.w = kb[SDL_SCANCODE_W] != 0;      in.s = kb[SDL_SCANCODE_S] != 0;
            in.a = kb[SDL_SCANCODE_A] != 0;      in.d = kb[SDL_SCANCODE_D] != 0;
            in.r = kb[SDL_SCANCODE_R] != 0;      in.f = kb[SDL_SCANCODE_F] != 0;
            in.left = kb[SDL_SCANCODE_LEFT] != 0;   in.right = kb[SDL_SCANCODE_RIGHT] != 0;
            in.up = kb[SDL_SCANCODE_UP] != 0;       in.down = kb[SDL_SCANCODE_DOWN] != 0;
        }
        SGE::Render::update(m_camera, in, frameDt);

        RenderScene();
        RenderDebugUi();
        m_pwindow->showWithOverlay([&](SDL_Renderer *r){
            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), r);
        });
        //std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    ShutdownUi();
    LOGI("Quit Normally");
    return {};
}