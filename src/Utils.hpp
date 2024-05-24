#include "WindowDefine.hpp"
#include "Window.hpp"
#include <random>
#include <ctime>

//Here is a trick code, I am too lazy to write all switch code
inline static WindowEventType WindowEventType2SDLEventType(const Uint32 ev){
    return static_cast<WindowEventType>(ev);
}

inline static void WriteBufferIntoSDLTexture(SDL_Texture *ptexture, const void *buffer, const std::size_t sz){
#if ENABLE_DRAWBUFFER_TEST
    FILE* fp = fopen("/home/ares/home/Code/SoftGameEngine/tmp/720x470.raw", "w");
    fwrite(buffer, sz, 1, fp);
    fclose(fp);
#endif
    void *pixels{};
    int pitch{};
    SDL_LockTexture(ptexture, nullptr, &pixels, &pitch);
    memcpy(pixels, buffer, sz);
    SDL_UnlockTexture(ptexture);
}

inline static SDL_PixelFormatEnum Format2SDLFormat(const RenderFormat format){
    switch(format){
        case RenderFormat::RGBA8888: return SDL_PIXELFORMAT_RGBA8888;
        case RenderFormat::BGRA8888: return SDL_PIXELFORMAT_BGRA8888;
    }

    return {};
}

inline static std::size_t FetchPackBytesAccordingFormat(const RenderFormat format){
    switch(format){
        case RenderFormat::RGBA8888: return 4;
        case RenderFormat::BGRA8888: return 4;
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

inline static Color GenerateColor(){
    auto r = GenerateRandomValue<uint8_t>(0, 255);
    auto g = GenerateRandomValue<uint8_t>(0, 255);
    auto b = GenerateRandomValue<uint8_t>(0, 255);
    return {r, g, b, 1};
}