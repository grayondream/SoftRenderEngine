
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <system_error>
#include <SDL2/SDL.h>

#include "Window.hpp"
#include "ErrorCode.hpp"
#include "Log.hpp"
#include "WindowDefine.hpp"


constexpr const static char *kGameEngineTitle = "My Soft Game Engine";

//Here is a trick code, I am too lazy to write all switch code
inline static WindowEventType WindowEventType2SDLEventType(const Uint32 ev){
    return static_cast<WindowEventType>(ev);
}

Window::Window(const PositionI32 pos)
    : m_pos(pos){

}

Window::~Window(){
    if(m_pWindow){
        SDL_DestroyWindow(m_pWindow);
    }

    if(m_pRender){
        SDL_DestroyRenderer(m_pRender);
    }

}

std::error_code Window::init(){
    if(!m_pWindow){
        m_pWindow = SDL_CreateWindow(kGameEngineTitle, static_cast<int>(m_pos.lf.x), static_cast<int>(m_pos.lf.y), static_cast<int>(m_pos.width()), static_cast<int>(m_pos.height()), 0);
    }
    
    if(!m_pWindow){
        LOGE("Failed to create sdl window!error message: {}", SDL_GetError());
        return std::error_code(static_cast<int>(ErrorCode::FAILED), std::generic_category());
    }

    if(!m_pRender){
        m_pRender = SDL_CreateRenderer(m_pWindow, -1, SDL_RENDERER_ACCELERATED);
    }

    if(!m_pRender){
        LOGE("Failed to create sdl render!error message: {}", SDL_GetError());
        return std::error_code(static_cast<int>(ErrorCode::FAILED), std::generic_category());
    }

    return {};
}

void Window::show(){
    SDL_SetRenderDrawColor(m_pRender, m_color.x, m_color.y, m_color.z, m_color.w);
    SDL_RenderClear(m_pRender);
    processEvent();
    SDL_RenderPresent(m_pRender);
}

void Window::processEvent(){
    if(!m_listener) return;

    SDL_Event ev{};
    while(SDL_PollEvent(&ev)){
        auto type = WindowEventType2SDLEventType(ev.type);
        (*m_listener)(type);
    }
}
