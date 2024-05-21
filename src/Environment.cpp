
#include <SDL2/SDL.h>
#include <system_error>
#include "Environment.hpp"
#include "BufferManager.hpp"
#include "ErrorCode.hpp"
#include "Log.hpp"
#include "WindowDefine.hpp"

Environment* Environment::instance(){
    static Environment env{};
    return &env;
}

std::error_code Environment::initalize(const EnvironmentParam &param){
    if(auto err = SDL_Init(SDL_INIT_VIDEO); err != 0){
        LOGE("Failed to initalize sdl, errorcode is {}!", err);
        return std::error_code(static_cast<int>(ErrorCode::SDL_INIT_ERROR), std::generic_category());
    }

    if(auto err = BufferManager::instance()->initialize(param.size)){
        LOGE("Failed to initalize surface buffer, errorcode is {}!", err.value());
        return std::error_code(static_cast<int>(ErrorCode::SURFACE_ERROR), std::generic_category());
    }

    return {};
}