

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
#include "imgui_impl_sdl2.h"

constexpr const static char *kGameEngineTitle = "My Soft Game Engine";


Window::Window(const Position pos, const RenderFormat format)
    :   m_pos(pos),
        m_format(format){

}

Window::~Window(){
    if(m_pTexture){
        SDL_DestroyTexture(m_pTexture);
        m_pTexture = nullptr;
    }

    if(m_pRender){
        SDL_DestroyRenderer(m_pRender);
        m_pRender = nullptr;
    }

    if(m_pWindow){
        SDL_DestroyWindow(m_pWindow);
        m_pWindow = nullptr;
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
        LOGE("accelerated renderer unavailable ({}), falling back to software", SDL_GetError());
        m_pRender = SDL_CreateRenderer(m_pWindow, -1, SDL_RENDERER_SOFTWARE);
    }
    if(m_pRender){
        SDL_RendererInfo info{};
        if(SDL_GetRendererInfo(m_pRender, &info) == 0){
            std::fprintf(stderr, "[display] renderer=%s flags=%08X\n", info.name, info.flags);
            for(uint32_t i = 0; i < info.num_texture_formats; i++){
                std::fprintf(stderr, "[display] tex fmt[%u]=%s\n", i, SDL_GetPixelFormatName(info.texture_formats[i]));
            }
        }
    }

    if(!m_pRender){
        LOGE("Failed to create sdl render!error message: {}", SDL_GetError());
        return std::error_code(static_cast<int>(ErrorCode::FAILED), std::generic_category());
    }

    if(!m_pTexture){
        m_pTexture = SDL_CreateTexture(m_pRender, Format2SDLFormat(m_format), SDL_TEXTUREACCESS_STREAMING, m_pos.size.width, m_pos.size.height);
    }

    if(!m_pTexture){
        LOGE("Failed to create screen texture! eeror message: {}", SDL_GetError());
        return std::error_code(static_cast<int>(ErrorCode::FAILED), std::generic_category());
    }

    return {};
}

void Window::show(){
    showWithOverlay(nullptr);
}

void Window::showWithOverlay(const std::function<void(SDL_Renderer*)> &overlay){
    SDL_SetRenderDrawColor(m_pRender, m_color.r, m_color.g, m_color.b, m_color.a);
    SDL_RenderClear(m_pRender);
    BufferManager::instance()->swap();
    auto buffer = BufferManager::instance()->getRawBuffer();
    draw(buffer);
    if(SDL_RenderCopy(m_pRender, m_pTexture, nullptr, nullptr) != 0){
        std::fprintf(stderr, "[display] RenderCopy failed: %s\n", SDL_GetError());
    }
    if(overlay){
        overlay(m_pRender);
    }
    SDL_RenderPresent(m_pRender);
}

void Window::processEvent(){
    if(!m_listener) return;

    SDL_Event ev{};
    while(SDL_PollEvent(&ev)){
        ImGui_ImplSDL2_ProcessEvent(&ev);
        auto type = SDLEventType2WindowEventType(ev.type);
        (*m_listener)(type);
    }
}

void Window::draw(const uint8_t *buffer){
    WriteBufferIntoSDLTexture(m_pTexture, buffer, m_pos.size.width, m_pos.size.height);
}
