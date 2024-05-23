#include <chrono>
#include <memory>
#include <system_error>
#include <thread>
#include "Application.hpp"
#include "BufferManager.hpp"
#include "Environment.hpp"
#include "ErrorCode.hpp"
#include "Window.hpp"
#include "Log.hpp"

std::error_code Application::initalize(const ApplicationParam &param){
    if(auto err = Environment::instance()->initalize(param.env); err){
        return err;
    }

    m_pwindow = std::make_shared<Window>(param.env.pos);
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
    BufferManager::instance()->clear({0, 0, 0, 1});
    while(!m_bQuit){
        m_pwindow->processEvent();
        m_pwindow->show();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    LOGI("Quit Normally");
    return {};
}