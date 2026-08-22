#include <chrono>
#include <cmath>
#include <cstdio>
#include <memory>
#include <ratio>
#include <system_error>
#include <thread>
#include <vector>
#include "Application.hpp"
#include "BufferManager.hpp"
#include "Environment.hpp"
#include "ErrorCode.hpp"
#include "Window.hpp"
#include "Log.hpp"
#include "WindowBuffer.hpp"
#include "Render/Rasterizer.hpp"
#include "Render/Pipeline.hpp"
#include "Render/Texture.hpp"
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

    m_cube = MakeCube();
    m_checker = MakeCheckerTexture();
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

void Application::RenderCube(){
    m_angle += 0.02;
    auto model = SGE::Math::translation(m_cube.worldPos.x, m_cube.worldPos.y, m_cube.worldPos.z)
        .mul(SGE::Math::rotationY(m_angle))
        .mul(SGE::Math::rotationX(0.4));
    auto view = SGE::Math::lookAt(Vector3DBase<double>{0, 2, -6},
                                  Vector3DBase<double>{0, 0, 0},
                                  Vector3DBase<double>{0, 1, 0});
    auto proj = SGE::Math::perspective(M_PI/3, 800.0/600.0, 0.1, 100.0);
    auto mvp = proj.mul(view).mul(model);

    m_framebuffer.clear(0xFF000000u);
    Rasterizer rz{m_framebuffer};
    for(auto &t : Pipeline::projectObject(m_cube, model, mvp, SGE::Math::normalMatrix(model), 800, 600)){
        rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2], m_checker);
    }

    if(auto buf = BufferManager::instance()->getBuffer()){
        buf->clear({0,0,0,255});
        buf->blitFrame(m_framebuffer.colorData(), m_framebuffer.width(), m_framebuffer.height());
    }
}

std::error_code Application::run(){
    if(!m_pwindow){
        LOGE("The window is nullptr, can not display any scene!");
        return std::error_code(static_cast<int>(ErrorCode::FAILED), std::generic_category());
    }

    m_pwindow->setListener(this);
    const std::chrono::high_resolution_clock::time_point pt = std::chrono::high_resolution_clock::now();
    int64_t delaTime = 1000;    //ms
    int64_t cycleCount = 0;
    BufferManager::instance()->clear(GenerateColor());
    while(!m_bQuit){
        cycleCount += 1;
        auto durationPerCycle = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - pt).count();
        if(durationPerCycle >= delaTime){
            auto fps = cycleCount * 1000.0/ durationPerCycle;
            m_pwindow->setTitle(std::format("Game Engine: Fps {:.2f}", fps));
        }

        m_pwindow->processEvent();
        RenderCube();
        m_pwindow->show();
        //std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    LOGI("Quit Normally");
    return {};
}