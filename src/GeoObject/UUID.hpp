#pragma once

#include <algorithm>
#include <uuid/uuid.h>
#include <string>

inline constexpr int kUUIDLen = 16;

class UUID{
    friend bool operator==(const UUID& rst, const UUID &snd);
    friend std::string to_string(const UUID &obj);
public:
    UUID(){
        uuid_generate(m_uuid);
    }

    UUID(const UUID &id){
        uuid_copy(m_uuid, id.m_uuid);
    }

    UUID(const UUID &&id){
        uuid_copy(m_uuid, id.m_uuid);
    }

    UUID& operator=(const UUID& id){
        uuid_copy(m_uuid, id.m_uuid);
        return *this;
    }


    UUID& operator=(const UUID&& id){
        uuid_copy(m_uuid, id.m_uuid);
        return *this;
    }

    void clear(){
        uuid_clear(m_uuid);
    }
private:
    uuid_t m_uuid{};
};

inline bool operator==(const UUID& rst, const UUID &snd){
    return 0 == uuid_compare(rst.m_uuid, snd.m_uuid);
}

inline bool operator!=(const UUID& rst, const UUID &snd){
    return !(rst == snd);
}

inline std::string to_string(const UUID &obj){
    std::string ret;
    ret.resize(kUUIDLen + 1);
    std::copy_n(obj.m_uuid, kUUIDLen, ret.data());
    ret.data()[kUUIDLen] = '\0';
    return ret;
}




