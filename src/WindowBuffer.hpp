#pragma once
#include <algorithm>
#include <vector>
#include "WindowDefine.hpp"
#include "Utils.hpp"

template<class T>
struct WindowBufferBase{
protected:
    //rgba 8bit
    std::vector<T> m_buffer{};
    Size m_size{};  
    std::size_t m_pitch{};  
};

template<class T>
struct WindowBuffer : public WindowBufferBase<T>{
public:
    WindowBuffer(const Size &sz, const RenderFormat format){
        this->m_size = sz;
        this->m_format = format;
        this->m_pitch = FetchPitchAccordingFormat(format, sz.width);
        this->m_buffer.resize(size(0));
    }

    bool fill(const T *buffer){
        if(locked()){
            return false;
        }

        lock();
        std::vector<T> vec{buffer, buffer + size(0)};
        this->m_buffer.swap(vec);
        unlock();
        return true;
    }

    bool clear(const WindowColor &color){
        if(locked()){
            return false;
        }

        lock();
        T* buffer = this->m_buffer.data();
        auto height = this->m_size.height;
        auto width  = this->m_size.width;
        auto pitch  = this->m_pitch;
        auto bytes = FetchPackBytesAccordingFormat(this->m_format);
        //32 位小端内存序：
        //RGBA8888: a b g r；BGRA8888: a r g b；ARGB8888: b g r a
        for(std::size_t i = 0;i < height;i ++){
            for(std::size_t j = 0;j < width;j ++){
                auto base = i * pitch + j * bytes;
                if(this->m_format == RenderFormat::BGRA8888){
                    buffer[base + 0] = color.a;
                    buffer[base + 1] = color.r;
                    buffer[base + 2] = color.g;
                    buffer[base + 3] = color.b;
                }else if(this->m_format == RenderFormat::ARGB8888){
                    buffer[base + 0] = color.b;
                    buffer[base + 1] = color.g;
                    buffer[base + 2] = color.r;
                    buffer[base + 3] = color.a;
                }else{
                    buffer[base + 0] = color.a;
                    buffer[base + 1] = color.b;
                    buffer[base + 2] = color.g;
                    buffer[base + 3] = color.r;
                }
            }
        }

        unlock();
        return true;
    }

    bool lock(){
        if(locked()){
            return false;
        }

        m_locked = true;
        return true;
    }

    bool unlock(){
        if(!locked()){
            return false;
        }

        m_locked = false;
        return true;
    }

    bool locked(){
        return m_locked;
    }

    Size size(){
        return this->m_size;
    }

    std::size_t size(const int){
        auto sz = size();
        return this->m_pitch * sz.height;
    }

    T* buffer(){
        return this->m_buffer.data();
    }

    bool blitFrame(const uint32_t *src, const std::size_t w, const std::size_t h){
        if(locked()){
            return false;
        }

        lock();
        uint8_t *dst = this->m_buffer.data();
        const auto bytes = FetchPackBytesAccordingFormat(this->m_format);
        //framebuffer 打包恒为 a<<24|r<<16|g<<8|b，小端内存即 [b,g,r,a]
        for(std::size_t i = 0;i < h;i ++){
            for(std::size_t j = 0;j < w;j ++){
                uint32_t v = src[i * w + j];
                auto base = i * this->m_pitch + j * bytes;
                dst[base + 0] = static_cast<uint8_t>(v & 0xFF);
                dst[base + 1] = static_cast<uint8_t>((v >> 8) & 0xFF);
                dst[base + 2] = static_cast<uint8_t>((v >> 16) & 0xFF);
                dst[base + 3] = static_cast<uint8_t>((v >> 24) & 0xFF);
            }
        }

        unlock();
        return true;
    }
public:
    bool m_locked{};
    RenderFormat m_format{};
};
