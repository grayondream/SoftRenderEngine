#pragma once
#include <algorithm>
#include <vector>
#include "WindowDefine.hpp"

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
    WindowBuffer(const Size &sz){
        this->m_size = sz;
        this->m_pitch = sz.width * 4;
        this->m_buffer.reserve(size(0));
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

    bool clear(const Color &color){
        if(locked()){
            return false;
        }

        lock();
        T* buffer = this->m_buffer.data();
        auto height = this->m_size.height;
        auto width  = this->m_size.width;
        auto pitch  = this->m_pitch;
        for(auto i = 0;i < height;i ++){
            for(auto j = 0;j < width;j ++){
                auto base = i * pitch + j * 4;
                buffer[base + 0] = color.w;
                buffer[base + 1] = color.z;
                buffer[base + 2] = color.y;
                buffer[base + 3] = color.x;
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
public:
    bool m_locked{};
};