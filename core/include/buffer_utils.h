#ifndef HAK_BUFFER_UTILS_H
#define HAK_BUFFER_UTILS_H

#include <sys/endian.h>
#include "types.h"

namespace hak {
    void int16_to_buf(i16 num, char* buf);

    void int32_to_buf(i32 num, char* buf);

    void int64_to_buf(i64 num, char* buf);

    void float_to_buf(float num, char* buf);

    void double_to_buf(double num, char* buf);

    auto buf_to_int16(const char* buf) -> i16;

    auto buf_to_int32(const char* buf) -> i32;

    auto buf_to_int64(const char* buf) -> i64;

    auto buf_to_float(const char* buf) -> float;

    auto buf_to_double(const char* buf) -> double;

    auto to_hex_string(const char* data, size_t len) -> std::string;
}

#endif //HAK_BUFFER_UTILS_H
