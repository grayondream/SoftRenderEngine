
#include <SDL2/SDL.h>
#include <system_error>
#include "Environment.hpp"
#include "ErrorCode.hpp"
#include "Log.hpp"

Environment* Environment::instance(){
    static Environment env{};
    return &env;
}

std::error_code Environment::initalize(){
    if(auto err = SDL_Init(SDL_INIT_VIDEO); err != 0){
        LOGE("Failed to initalize sdl, errorcode is {}!", err);
        return std::error_code(static_cast<int>(ErrorCode::SDL_INIT_ERROR), std::generic_category());
    }

    return {};
}