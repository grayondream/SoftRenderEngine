#pragma once
#include <system_error>
#include <memory>
#include "Environment.hpp"
#include "Window.hpp"
#include "WindowDefine.hpp"
#include "Render/FrameBuffer.hpp"
#include "Render/GeoObject/Object4D.hpp"
#include "Render/Texture.hpp"
#include "Render/Camera.hpp"
#include "Render/Light.hpp"
#include "Render/RayTrace.hpp"

struct ApplicationParam{
    EnvironmentParam env{};
};

class Application : public WindowEventListener{
public:
    std::error_code initalize(const ApplicationParam &param);

    std::error_code run();

    virtual void operator()(const WindowEventType t) override final;

private:
    void RenderScene();
    void RenderDebugUi();
    void ShutdownUi();

    std::shared_ptr<Window> m_pwindow{};
    bool m_bQuit{false};
    FrameBuffer m_framebuffer{800, 600};
    Object4D m_cube{};
    Texture m_checker{};
    double m_angle{0.0};
    SGE::Render::Camera m_camera{};

    int m_sceneMode{0};
    bool m_rotating{true};
    float m_rotateSpeed{0.02f};
    bool m_fogEnabled{true};
    float m_fogStart{8.0f};
    float m_fogEnd{25.0f};
    LightingRig m_rig{};
    SGE::Render::RayScene m_rtScene{};
    FrameBuffer m_rtBuffer{200, 150};
    bool m_bracketHeld{false};
};
