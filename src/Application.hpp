#pragma once
#include <system_error>
#include <memory>
#include "Environment.hpp"
#include "Window.hpp"
#include "WindowDefine.hpp"
#include "Render/FrameBuffer.hpp"
#include "Render/GeoObject/Object4D.hpp"

struct ApplicationParam{
    EnvironmentParam env{};
};

class Application : public WindowEventListener{
public:
    std::error_code initalize(const ApplicationParam &param);

    std::error_code run();

    virtual void operator()(const WindowEventType t) override final;

private:
    void RenderCube();

    std::shared_ptr<Window> m_pwindow{};
    bool m_bQuit{false};
    FrameBuffer m_framebuffer{800, 600};
    Object4D m_cube{};
    double m_angle{0.0};
};