#include <cstdint>
#include <memory>
#include <system_error>
#include <cassert>
#include "BufferManager.hpp"
#include "ErrorCode.hpp"
#include "WindowBuffer.hpp"
#include "WindowDefine.hpp"
#include "Log.hpp"

std::error_code BufferManager::initialize(const Size &sz, const RenderFormat format){
    m_primaryBuffer  = std::make_shared<WindowBuffer<uint8_t>>(sz, format);
    m_auxilaryBuffer = std::make_shared<WindowBuffer<uint8_t>>(sz, format);
    return {};
}

std::error_code BufferManager::draw(const uint8_t *buffer){
    if(!m_auxilaryBuffer->fill(buffer)){
        LOGE("can not fill screen buffer!");
        return std::error_code(static_cast<int>(ErrorCode::FATAL_ERROR), std::generic_category());
    }

    m_bNeedSwap = true;
    return {};
}

uint8_t* BufferManager::getRawBuffer(){
    assert(!m_primaryBuffer->locked());
    return m_primaryBuffer->buffer();
}

std::shared_ptr<BufferManager::BufferType> BufferManager::getBuffer(){
    return m_auxilaryBuffer;
}
void BufferManager::clear(const Color &color){
    assert(!m_auxilaryBuffer->locked());
    m_auxilaryBuffer->clear(color);
    m_bNeedSwap = true;
}

void BufferManager::swap(){
    if(!m_bNeedSwap){
        return;
    }

    assert(!m_auxilaryBuffer->locked() && !m_primaryBuffer->locked());
    m_auxilaryBuffer.swap(m_primaryBuffer);
    m_bNeedSwap = false;
}