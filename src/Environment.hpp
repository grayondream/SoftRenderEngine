#pragma once
#include "WindowDefine.hpp"
#include<system_error>

struct EnvironmentParam{
    Position pos;
    RenderFormat format;
};

class Environment{
private:
    Environment(){};
    Environment(const Environment&){}

public:
    static Environment* instance();

public:
    std::error_code initalize(const EnvironmentParam &param);
};