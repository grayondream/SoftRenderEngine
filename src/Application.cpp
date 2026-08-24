#include <chrono>
#include <random>
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
#include "Render/SortUtil.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "Transform.hpp"
#include "Samples/IScene.hpp"

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

LightingRig makeDefaultRigForScene(){
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

}

Application::Application(){
    if(const char *sc = std::getenv("SGE_START_SCENE")){
        m_sceneIndex = std::atoi(sc);
        m_sceneDirty = true;
    }
}
Application::~Application() = default;

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
    SGE::Samples::registerBuiltinScenes();
    auto &entries = SGE::Samples::SceneRegistry::instance().entries();
    if(entries.empty()){
        return;
    }
    if(m_sceneDirty || !m_scene){
        if(m_sceneIndex < 0 || m_sceneIndex >= static_cast<int>(entries.size())){
            m_sceneIndex = 0;
        }
        m_scene = entries[m_sceneIndex].factory();
        m_scene->setup(*this);
        m_sceneDirty = false;
    }
    // frame-skip: reuse the last rendered frame for heavy scenes
    if(m_renderEveryN > 1 && m_lastFrameValid && !m_sceneDirty
       && (m_frameCounter++ % m_renderEveryN) != 0){
        BufferManager::instance()->draw(
            reinterpret_cast<const uint8_t*>(m_framebuffer.colorData()));
        return;
    }
    if(m_rotating){
        m_angle += m_rotateSpeed;
    }
    m_rig = makeDefaultRigForScene();
    m_scene->render(*this);
    m_lastFrameValid = true;

    BufferManager::instance()->draw(
        reinterpret_cast<const uint8_t*>(m_framebuffer.colorData()));
}

void Application::RenderDebugUi(){
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("SoftEngine Debug");
    SGE::Samples::registerBuiltinScenes();
    auto &entries = SGE::Samples::SceneRegistry::instance().entries();
    static std::vector<std::string> labels;
    static std::vector<const char*> labelPtrs;
    if(labels.size() != entries.size()){
        labels.clear();
        labelPtrs.clear();
        for(const auto &e : entries){
            std::string label = std::string("[") + e.group + "] " + e.name;
            labels.push_back(label);
        }
        for(auto &l : labels){ labelPtrs.push_back(l.c_str()); }
    }
    if(ImGui::Combo("Scene", &m_sceneIndex,
                    labelPtrs.data(), static_cast<int>(labelPtrs.size()))){
        LOGI("[UI] scene -> {} via ImGui", m_sceneIndex);
        m_framebuffer.clearDepth();
        m_sceneDirty = true;
    }
    if(m_sceneIndex >= 0 && m_sceneIndex < static_cast<int>(entries.size())){
        ImGui::Text("Current: %s", entries[m_sceneIndex].name);
    }
    ImGui::SliderInt("Render Every N", &m_renderEveryN, 1, 6);
    if(m_renderEveryN > 1){
        ImGui::Text("(~%.0f fps effective)", 60.0f / m_renderEveryN);
    }
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
    if(m_scene){
        m_scene->drawUi(*this);
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
        static bool s_f12Held = false;
        if(kb && kbCount > 0 && kb[SDL_SCANCODE_F12] && !s_f12Held){
            const std::string path = std::format("sge_screenshot_{}.ppm",
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count());
            FILE *out = std::fopen(path.c_str(), "wb");
            if(out){
                std::fprintf(out, "P6\n%zu %zu\n255\n", m_framebuffer.width(), m_framebuffer.height());
                const auto *px = m_framebuffer.colorData();
                for(std::size_t i = 0; i < m_framebuffer.width() * m_framebuffer.height(); i++){
                    const unsigned char bgr[3] = {
                        static_cast<unsigned char>(px[i] & 0xFF),
                        static_cast<unsigned char>((px[i] >> 8) & 0xFF),
                        static_cast<unsigned char>((px[i] >> 16) & 0xFF)};
                    std::fwrite(bgr, 1, 3, out);
                }
                std::fclose(out);
                LOGI("[shot] saved {}", path);
            }
        }
        s_f12Held = kb && kbCount > 0 && kb[SDL_SCANCODE_F12];

        const int sceneCount = static_cast<int>(
            SGE::Samples::SceneRegistry::instance().entries().size());
        if(kb && kbCount > 0 && kb[SDL_SCANCODE_RIGHTBRACKET] && !m_bracketHeld){
            m_sceneIndex = (m_sceneIndex + 1) % std::max(1, sceneCount);
            m_framebuffer.clearDepth();
            m_sceneDirty = true;
            LOGI("[KEY] scene -> {} via ]", m_sceneIndex);
        }
        if(kb && kbCount > 0 && kb[SDL_SCANCODE_LEFTBRACKET] && !m_bracketHeld){
            m_sceneIndex = (m_sceneIndex + std::max(1, sceneCount) - 1)
                % std::max(1, sceneCount);
            m_framebuffer.clearDepth();
            m_sceneDirty = true;
            LOGI("[KEY] scene -> {} via [", m_sceneIndex);
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