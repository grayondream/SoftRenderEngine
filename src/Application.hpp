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
#include "Render/Primitives.hpp"

namespace SGE::Samples {
class IScene;
}

struct ApplicationParam{
    EnvironmentParam env{};
};

class Application : public WindowEventListener{
public:
    Application();
    ~Application();
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    std::error_code initalize(const ApplicationParam &param);

    std::error_code run();

    virtual void operator()(const WindowEventType t) override final;

    FrameBuffer &framebuffer(){ return m_framebuffer; }
    SGE::Render::Camera &camera(){ return m_camera; }
    LightingRig &rig(){ return m_rig; }
    double &angle(){ return m_angle; }
    bool &rotating(){ return m_rotating; }
    float &rotateSpeed(){ return m_rotateSpeed; }
    bool &fogEnabled(){ return m_fogEnabled; }
    float &fogStart(){ return m_fogStart; }
    float &fogEnd(){ return m_fogEnd; }
    Texture &checker(){ return m_checker; }
    Object4D &cube(){ return m_cube; }
    Object4D &sphere(){ return m_sphere; }
    Object4D &torus(){ return m_torus; }
    Object4D &teapot(){ return m_teapot; }
    float &pbrMetallic(){ return m_pbrMetallic; }
    float &pbrRoughness(){ return m_pbrRoughness; }
    float *pbrColorUi(){ return m_pbrColorUi; }
    Color32 &pbrBase(){ return m_pbrBase; }
    int &pcfRadius(){ return m_pcfRadius; }

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
    Object4D m_sphere{};
    Object4D m_torus{};
    Object4D m_teapot{};

    std::unique_ptr<SGE::Samples::IScene> m_scene{};
    int m_sceneIndex{-1};
    bool m_sceneDirty{true};
    int m_renderEveryN{1};
    int m_renderScalePct{100};
    int m_frameCounter{0};
    bool m_lastFrameValid{false};
    bool m_rotating{true};
    float m_rotateSpeed{0.02f};
    bool m_fogEnabled{true};
    float m_fogStart{8.0f};
    float m_fogEnd{25.0f};
    LightingRig m_rig{};
    bool m_bracketHeld{false};
    float m_pbrMetallic{0.0f};
    float m_pbrColorUi[3]{0.78f, 0.24f, 0.24f};
    float m_pbrRoughness{0.35f};
    Color32 m_pbrBase{200, 60, 60, 255};
    int m_pcfRadius{2};
};
