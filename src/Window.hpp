#pragma once
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <cstdint>
#include <system_error>
#include "WindowDefine.hpp"

enum class WindowEventType : uint32_t{
    WINDOW_EVENT_QUIT = 0x100,
};


class WindowEventListener{
public:
    virtual void operator()(const WindowEventType type) = 0;
};

struct SDL_Renderer;
struct SDL_Window;
class Window{
public:
    Window(const Position pos = {{WINDOW_DEFAULT_X, WINDOW_DEFAULT_Y}, {WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT}}, const RenderFormat format = RenderFormat::RGBA8888);
    ~Window();

    Window(const Window &) = delete;
    Window& operator=(const Window &) = delete;

    Window(Window &&other) noexcept
        : m_pos(other.m_pos),
          m_format(other.m_format),
          m_color(other.m_color),
          m_pRender(other.m_pRender),
          m_pWindow(other.m_pWindow),
          m_pTexture(other.m_pTexture),
          m_listener(other.m_listener){
        other.m_pRender = nullptr;
        other.m_pWindow = nullptr;
        other.m_pTexture = nullptr;
        other.m_listener = nullptr;
    }

    Window& operator=(Window &&other) noexcept{
        if(this != &other){
            this->~Window();
            m_pos = other.m_pos;
            m_format = other.m_format;
            m_color = other.m_color;
            m_pRender = other.m_pRender;
            m_pWindow = other.m_pWindow;
            m_pTexture = other.m_pTexture;
            m_listener = other.m_listener;
            other.m_pRender = nullptr;
            other.m_pWindow = nullptr;
            other.m_pTexture = nullptr;
            other.m_listener = nullptr;
        }
        return *this;
    }
    
public:
    std::error_code init();

    void setBackgroundColor(const Color &color){
        m_color = color;
    }

    void setTitle(const std::string &title){
        SDL_SetWindowTitle(m_pWindow, title.c_str());
    }

    void show();

    WindowIDType getId(){
        return reinterpret_cast<WindowIDType>(this);
    }

    void setListener(WindowEventListener *ls){
        m_listener = ls;
    }

    void processEvent();

    void draw(const uint8_t* buffer);

private:

private:
    Position m_pos{};
    RenderFormat m_format{};
    Color  m_color{128, 128, 128, 128};
    SDL_Renderer* m_pRender{};
    SDL_Window* m_pWindow{};
    SDL_Texture* m_pTexture{};
    WindowEventListener *m_listener{};
};