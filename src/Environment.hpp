#pragma once
#include<system_error>

class Environment{
private:
    Environment(){};
    Environment(const Environment&){}

public:
    static Environment* instance();

public:
    std::error_code initalize();
};