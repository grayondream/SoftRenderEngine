#pragma once
#include "WindowDefine.hpp"
#include "Window.hpp"
#include <cstring>
#include <random>
#include <cstdio>
#include <ctime>

//Here is a trick code, I am too lazy to write all switch code
inline static WindowEventType SDLEventType2WindowEventType(const Uint32 ev){
    return static_cast<WindowEventType>(ev);
}

inline static void WriteBufferIntoSDLTexture(SDL_Texture *ptexture, const void *buffer, const std::size_t width, const std::size_t height){
    void *pixels{};
    int pitch{};
    if(SDL_LockTexture(ptexture, nullptr, &pixels, &pitch) != 0){
        std::fprintf(stderr, "[display] LockTexture failed: %s\n", SDL_GetError());
        return;
    }

    const auto *src = static_cast<const uint8_t*>(buffer);
    auto *dst = static_cast<uint8_t*>(pixels);
    const auto bytesPerRow = width * 4;
    for(std::size_t row = 0; row < height; ++row){
        std::memcpy(dst + static_cast<std::size_t>(pitch) * row, src + bytesPerRow * row, bytesPerRow);
    }

    SDL_UnlockTexture(ptexture);
}

inline static SDL_PixelFormatEnum Format2SDLFormat(const RenderFormat format){
    switch(format){
        case RenderFormat::RGBA8888: return SDL_PIXELFORMAT_RGBA8888;
        case RenderFormat::BGRA8888: return SDL_PIXELFORMAT_BGRA8888;
        case RenderFormat::ARGB8888: return SDL_PIXELFORMAT_ARGB8888;
    }

    return {};
}

inline static std::size_t FetchPackBytesAccordingFormat(const RenderFormat format){
    switch(format){
        case RenderFormat::RGBA8888: return 4;
        case RenderFormat::BGRA8888: return 4;
        case RenderFormat::ARGB8888: return 4;
    }

    return {};
}

inline static std::size_t FetchPitchAccordingFormat(const RenderFormat format, const std::size_t width){
    return width * FetchPackBytesAccordingFormat(format);
}


template<class T>
T GenerateRandomValue(const T minv, const T maxv){
    std::mt19937 rng(std::time(nullptr));
    // Define the distribution
    std::uniform_int_distribution<T> distribution(minv, maxv);
    // Generate and return the random integer
    return distribution(rng);
}

inline static WindowColor GenerateColor(){
    auto r = GenerateRandomValue<uint8_t>(0, 255);
    auto g = GenerateRandomValue<uint8_t>(0, 255);
    auto b = GenerateRandomValue<uint8_t>(0, 255);
    return {r, g, b, 255};
}