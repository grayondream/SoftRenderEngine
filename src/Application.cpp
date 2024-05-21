#include <memory>
#include <system_error>
#include "Application.hpp"
#include "BufferManager.hpp"
#include "Environment.hpp"
#include "ErrorCode.hpp"
#include "Window.hpp"
#include "Log.hpp"
#include "WindowDefine.hpp"

std::error_code Application::initalize(const ApplicationParam &param){
    if(auto err = Environment::instance()->initalize(param.env); err){
        return err;
    }

    m_pwindow = std::make_shared<Window>(Point2D{100, 100}, param.env.size);
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
    while(!m_bQuit){
        BufferManager::instance()->
        BufferManager::instance()->swap();
        m_pwindow->show();
    }

    LOGI("Quit Normally");
    return {};
}