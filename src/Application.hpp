#pragma once
#include <system_error>
#include <memory>
#include "Environment.hpp"
#include "Window.hpp"
#include "WindowDefine.hpp"

struct ApplicationParam{
    EnvironmentParam env{};
};

class Application : public WindowEventListener{
public:
    std::error_code initalize(const ApplicationParam &param);

    std::error_code run();

    virtual void operator()(const WindowEventType t) override final;

private:
    std::shared_ptr<Window> m_pwindow{};
    bool m_bQuit{false};
};