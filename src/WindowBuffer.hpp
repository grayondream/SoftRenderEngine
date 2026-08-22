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
        //SDL 像素格式为 32 位小端存储：
        //RGBA8888 内存顺序为 a b g r；BGRA8888 内存顺序为 a r g b
        const bool bgra = (this->m_format == RenderFormat::BGRA8888);
        for(auto i = 0;i < height;i ++){
            for(auto j = 0;j < width;j ++){
                auto base = i * pitch + j * bytes;
                buffer[base + 0] = color.a;
                if(bgra){
                    buffer[base + 1] = color.r;
                    buffer[base + 2] = color.g;
                    buffer[base + 3] = color.b;
                }else{
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
        const bool bgra = (this->m_format == RenderFormat::BGRA8888);
        const auto cw = std::min(w, this->m_size.width);
        const auto ch = std::min(h, this->m_size.height);
        for(std::size_t i = 0;i < ch;i ++){
            for(std::size_t j = 0;j < cw;j ++){
                uint32_t v = src[i * w + j];
                auto base = i * this->m_pitch + j * bytes;
                dst[base + 0] = static_cast<uint8_t>((v >> 24) & 0xFF);
                if(bgra){
                    dst[base + 1] = static_cast<uint8_t>((v >> 16) & 0xFF);
                    dst[base + 2] = static_cast<uint8_t>((v >> 8) & 0xFF);
                    dst[base + 3] = static_cast<uint8_t>(v & 0xFF);
                }else{
                    dst[base + 1] = static_cast<uint8_t>(v & 0xFF);
                    dst[base + 2] = static_cast<uint8_t>((v >> 8) & 0xFF);
                    dst[base + 3] = static_cast<uint8_t>((v >> 16) & 0xFF);
                }
            }
        }

        unlock();
        return true;
    }
public:
    bool m_locked{};
    RenderFormat m_format{};
};
