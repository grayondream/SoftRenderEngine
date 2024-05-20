#pragma once

#include <cstdint>
enum class ErrorCode : int32_t{
    SUCCESS = 0,
    FAILED = -1,
    SDL_INIT_ERROR = -2,
};