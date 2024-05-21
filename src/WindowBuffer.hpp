#pragma once
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
        this->m_pitch = sz.width;
        this->m_buffer.reserve(size(0));
    }

    bool fill(const T *buffer){
        if(locked()){
            return false;
        }

        std::vector<T> vec{buffer, buffer + size(0)};
        this->m_buffer.swap(vec);
        return true;
    }

    bool clear(const Color &color){
        if(lock()){
            return false;
        }

        T* buffer = this->m_buffer.data();
        auto height = this->m_size.height;
        auto width  = this->m_size.width;
        for(int i = 0;i < height; i ++){
            for(int j = 0;j < width;j ++){
                auto base = i * height + j;
                buffer[base + 0] = color.x;
                buffer[base + 1] = color.y;
                buffer[base + 2] = color.z;
                buffer[base + 3] = color.w;
            }
        }
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
    }

    bool locked(){
        return m_locked;
    }

    Size size(){
        return this->m_size;
    }

    std::size_t size(const int){
        auto sz = size();
        return this->m_pitch * sz.height * 4;
    }

    T* buffer(){
        return this->m_buffer.data();
    }
public:
    bool m_locked{};
};