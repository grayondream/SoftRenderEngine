#include <cstdint>
#include <memory>
#include <system_error>
#include <cassert>
#include "BufferManager.hpp"
#include "ErrorCode.hpp"
#include "WindowBuffer.hpp"
#include "WindowDefine.hpp"
#include "Log.hpp"

std::error_code BufferManager::initialize(const Size &sz){
    m_primaryBuffer  = std::make_shared<WindowBuffer<uint8_t>>(sz);
    m_auxilaryBuffer = std::make_shared<WindowBuffer<uint8_t>>(sz);
    return {};
}

std::error_code BufferManager::draw(const uint8_t *buffer){
    if(!m_auxilaryBuffer->fill(buffer)){
        LOGE("can not fill screen buffer!");
        return std::error_code(static_cast<int>(ErrorCode::FATAL_ERROR), std::generic_category());
    }

    return {};
}

uint8_t* BufferManager::getBuffer(){
    assert(!m_primaryBuffer->lock());
    return m_primaryBuffer->buffer();
}

void BufferManager::clear(const Color &color){
    assert(!m_auxilaryBuffer->locked());
    m_auxilaryBuffer->clear(color);
}

void BufferManager::swap(){
    assert(!m_auxilaryBuffer->locked() && !m_primaryBuffer->locked());
    m_auxilaryBuffer.swap(m_primaryBuffer);
}