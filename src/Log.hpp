#pragma once

#include <iostream>
#include <format>
#include <string>

namespace sys{
    namespace log{
        template<typename ...Args>
        void Logg(std::format_string<Args...> fmt, Args&& ...args) {
            std::cout << std::format(fmt, std::forward<Args>(args)...) << std::endl;
        }

    }
}


using sys::log::Logg;

#define LOGI Logg
#define LOGW Logg
#define LOGE Logg
