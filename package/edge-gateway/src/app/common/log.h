#pragma once

#include <cstdio>
#include <ctime>

#define EG_LOG(level, fmt, ...)                                                     \
    do {                                                                            \
        std::time_t _now = std::time(nullptr);                                      \
        std::tm *_tm = std::localtime(&_now);                                       \
        char _buf[32];                                                              \
        std::strftime(_buf, sizeof(_buf), "%F %T", _tm);                           \
        std::fprintf(stderr, "[%s] [%s] " fmt "\n", _buf, level, ##__VA_ARGS__);    \
    } while (0)

#define EG_INFO(fmt, ...) EG_LOG("INFO", fmt, ##__VA_ARGS__)
#define EG_WARN(fmt, ...) EG_LOG("WARN", fmt, ##__VA_ARGS__)
#define EG_ERROR(fmt, ...) EG_LOG("ERROR", fmt, ##__VA_ARGS__)
