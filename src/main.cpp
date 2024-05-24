#include <iostream>
#include "Log.hpp"
#include "Application.hpp"
#include "WindowDefine.hpp"

int main(int argc, char **argv){
    LOGI("Hello Soft Game Engine");
    Application app{};
    ApplicationParam param{};
    param.env.pos = {{0, 0}, {1080, 720}};
    param.env.format = RenderFormat::RGBA8888;
    if(app.initalize(param)){
        LOGE("Failed to initalize application!");
        exit(-1);
    }

    if(app.run()){
        LOGE("Failed to run application!");
        exit(-1);
    }

    return 0;
}
