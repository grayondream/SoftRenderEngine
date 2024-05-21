#include <SDL/SDL_video.h>
#include <iostream>
#include "Log.hpp"
#include "Application.hpp"

int main(int argc, char **argv){
    LOGI("Hello Soft Game Engine");
    Application app{};
    ApplicationParam param{};
    param.env.size = {720, 480};
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
