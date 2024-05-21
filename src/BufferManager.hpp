#pragma once

#include "WindowDefine.hpp"
#include <cstdint>
#include <memory>
#include <system_error>
template<class T>
class WindowBuffer;

class BufferManager{
public:
    static BufferManager* instance(){
        static BufferManager instance;
        return &instance;
    }

    std::error_code initialize(const Size &sz);

    std::error_code draw(const uint8_t *buffer);

    void clear(const Color &color);

    void swap();

    uint8_t* getBuffer();

private:
    BufferManager() = default;

private:
    std::shared_ptr<WindowBuffer<uint8_t>> m_primaryBuffer;
    std::shared_ptr<WindowBuffer<uint8_t>> m_auxilaryBuffer;
};