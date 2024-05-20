#include <SDL/SDL_video.h>
#include <iostream>
#include "Log.hpp"
#include "Application.hpp"

int main(int argc, char **argv){
    LOGI("Hello Soft Game Engine");
    Application app{};
    if(app.initalize()){
        LOGE("Failed to initalize application!");
        exit(-1);
    }

    if(app.run()){
        LOGE("Failed to run application!");
        exit(-1);
    }

    return 0;
}
