

#include <SDL2/SDL_pixels.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <system_error>
#include <SDL2/SDL.h>

#include "Window.hpp"
#include "BufferManager.hpp"
#include "ErrorCode.hpp"
#include "Log.hpp"
#include "WindowDefine.hpp"
#include "Utils.hpp"

constexpr const static char *kGameEngineTitle = "My Soft Game Engine";


Window::Window(const Position pos, const RenderFormat format)
    :   m_pos(pos),
        m_format(format){

}

Window::~Window(){
    if(m_pTexture){
        SDL_DestroyTexture(m_pTexture);
    }

    if(m_pWindow){
        SDL_DestroyWindow(m_pWindow);
    }

    if(m_pRender){
        SDL_DestroyRenderer(m_pRender);
    }

}

std::error_code Window::init(){
    if(!m_pWindow){
        m_pWindow = SDL_CreateWindow(kGameEngineTitle, static_cast<int>(m_pos.lf.x), static_cast<int>(m_pos.lf.y), static_cast<int>(m_pos.size.width), static_cast<int>(m_pos.size.height), 0);
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

    if(!m_pTexture){
        m_pTexture = SDL_CreateTexture(m_pRender, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, m_pos.size.width, m_pos.size.height);
    }

    if(!m_pTexture){
        LOGE("Failed to create screen texture! eeror message: {}", SDL_GetError());
        return std::error_code(static_cast<int>(ErrorCode::FAILED), std::generic_category());
    }

    return {};
}

void Window::show(){
    SDL_SetRenderDrawColor(m_pRender, m_color.x, m_color.y, m_color.z, m_color.w);
    SDL_RenderClear(m_pRender);
    BufferManager::instance()->swap();
    auto buffer = BufferManager::instance()->getRawBuffer();
    draw(buffer);
    SDL_RenderCopy(m_pRender, m_pTexture, nullptr, nullptr);
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

void Window::draw(const uint8_t *buffer){
    WriteBufferIntoSDLTexture(m_pTexture, buffer, m_pos.size.width * m_pos.size.height * 4);
}
