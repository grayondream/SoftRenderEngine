#pragma once
#include <system_error>
#include "WindowDefine.hpp"

enum class WindowEventType : uint32_t{
    WINDOE_EVENT_QUIT = 0x100,
};


class WindowEventListener{
public:
    virtual void operator()(const WindowEventType type) = 0;
};

struct SDL_Renderer;
struct SDL_Window;
class Window{
public:
    Window(const Position pos = {{WINDOW_DEFAULT_X, WINDOW_DEFAULT_Y}, {WINDOW_DEFAULT_X + WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_Y + WINDOW_DEFAULT_HEIGHT}});
    Window(const Point2D& pt, const Size& sz){
        m_pos.lf = pt;
        m_pos.rb.x = pt.x + sz.width;
        m_pos.rb.y = pt.y + sz.height;
    }

    ~Window();
    
public:
    std::error_code init();

    void setBackgroundColor(const Color &color){
        m_color = color;
    }

    void show();

    WindowIDType getId(){
        return reinterpret_cast<WindowIDType>(this);
    }

    void setListener(WindowEventListener *ls){
        m_listener = ls;
    }

private:
    void processEvent();

private:
    Position m_pos{};
    Color  m_color{128, 128, 128, 128};
    SDL_Renderer* m_pRender{};
    SDL_Window* m_pWindow{};
    WindowEventListener *m_listener{};
};