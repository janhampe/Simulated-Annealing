#pragma once
#include <iostream>


template<typename... Args>
void debug(Args&&... args) {
    (std::cerr << ... << args) << std::endl;
}


#ifdef INFO_LOG
#define LOG_INFO(...) debug("INFO: ", __VA_ARGS__);
#else
#define LOG_INFO(...)
#endif

#ifdef ERROR_LOG
#define ERROR(...) debug("ERROR: ", __VA_ARGS__);
#else
#define ERROR(...)
#endif

#ifdef DEBUG_LOG
#define DEBUG(...) debug("DEBUG: ", __VA_ARGS__);
// #define DEBUG(...) std::cerr << __VA_ARGS__ << std::endl;
#else
#define DEBUG(...)
#endif
