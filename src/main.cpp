#include <iostream>
#include "Log.hpp"
#include "Application.hpp"
#include "WindowDefine.hpp"

int main(int, char **){
    LOGI("Hello Soft Game Engine");
    Application app{};
    ApplicationParam param{};
    param.env.pos = {{0, 0}, {800, 600}};
    param.env.format = RenderFormat::RGBA8888;
    if(app.initalize(param)){
        LOGE("Failed to initalize application!");
        return 1;
    }

    if(app.run()){
        LOGE("Failed to run application!");
        return 1;
    }

    return 0;
}
