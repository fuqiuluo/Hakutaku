#include "buffer_utils.h"

#include <sys/endian.h>
#include <sstream>
#include <iomanip>

namespace hak {
    void int16_to_buf(i16 num, char *buf) {
        int16_t tmp = htons(num);
        memcpy(buf, &tmp, sizeof(tmp));
    }

    void int32_to_buf(i32 num, char *buf) {
        int32_t tmp = htonl(num);
        memcpy(buf, &tmp, sizeof(tmp));
    }

    void int64_to_buf(i64 num, char *buf) {
        int64_t tmp = htobe64(num);
        memcpy(buf, &tmp, sizeof(tmp));
    }

    void float_to_buf(float num, char *buf) {
        union {
            i64 i;
            float f;
        } tmp;
        tmp.f = num;
        int64_to_buf(tmp.i, buf);
    }

    void double_to_buf(double num, char *buf) {
        union {
            i64 i;
            double d;
        } tmp;
        tmp.d = num;
        int64_to_buf(tmp.i, buf);
    }

    auto buf_to_int16(const char *buf) -> i16 {
        int16_t tmp = *(int16_t*)buf;
        return ntohs(tmp);
    }

    auto buf_to_int32(const char *buf) -> i32 {
        int32_t tmp = *(int32_t*)buf;
        return ntohl(tmp);
    }

    auto buf_to_int64(const char *buf) -> i64 {
        uint64_t tmp = *(uint64_t*)buf;
        return be64toh(tmp);
    }

    auto buf_to_float(const char *buf) -> float {
        union {
            i64 i;
            float f;
        } temp;
        uint64_t tmp = *(uint64_t*)buf;
        temp.i = be64toh(tmp);
        return temp.f;
    }

    auto buf_to_double(const char *buf) -> double {
        union {
            i64 i;
            double d;
        } temp;
        uint64_t tmp = *(uint64_t*)buf;
        temp.i = be64toh(tmp);
        return temp.d;
    }

    auto to_hex_string(const char* data, size_t len) -> std::string {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (size_t i = 0; i < len; ++i) {
            oss << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(data[i]));
        }
        return oss.str();
    }
}
