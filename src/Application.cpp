#include <chrono>
#include <memory>
#include <ratio>
#include <system_error>
#include <thread>
#include "Application.hpp"
#include "BufferManager.hpp"
#include "Environment.hpp"
#include "ErrorCode.hpp"
#include "Window.hpp"
#include "Log.hpp"
#include "WindowBuffer.hpp"

std::error_code Application::initalize(const ApplicationParam &param){
    if(auto err = Environment::instance()->initalize(param.env); err){
        return err;
    }

    m_pwindow = std::make_shared<Window>(param.env.pos, param.env.format);
    return m_pwindow->init();
}

 void Application::operator()(const WindowEventType t){
    switch(t){
        case WindowEventType::WINDOE_EVENT_QUIT:
            m_bQuit = true;
        default:
            LOGE("Not handle {}", static_cast<int>(t));
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
        m_pwindow->show();
        //std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    LOGI("Quit Normally");
    return {};
}