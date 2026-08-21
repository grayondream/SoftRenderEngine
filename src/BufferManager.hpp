#pragma once

#include "WindowDefine.hpp"
#include <cstdint>
#include <memory>
#include <system_error>
template<class T>
class WindowBuffer;

class BufferManager{
public:
    using BufferType = WindowBuffer<uint8_t>;
public:
    static BufferManager* instance(){
        static BufferManager instance;
        return &instance;
    }

    std::error_code initialize(const Size &sz, const RenderFormat format);

    std::error_code draw(const uint8_t *buffer);

    void clear(const WindowColor &color);

    void swap();

    uint8_t* getRawBuffer();

    std::shared_ptr<BufferType> getBuffer();

private:
    BufferManager() = default;

private:
    std::shared_ptr<BufferType> m_primaryBuffer;
    std::shared_ptr<BufferType> m_auxilaryBuffer;
    bool m_bNeedSwap{ false };
};