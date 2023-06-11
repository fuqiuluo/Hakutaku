#ifndef READER_H
#define READER_H

#include "types.h"
#include "process.h"

#include <vector>
#include <string>

namespace hak {
    class memory_reader {
        std::shared_ptr<hak::process> process;
    public:
        explicit memory_reader(std::shared_ptr<hak::process> proc);

        void read(pointer address, void* data, size_t size);

        template<typename T>
        auto read(pointer address) -> T;

        auto read_i8(pointer address) -> i8;

        auto read_i16(pointer address) -> i16;

        auto read_i32(pointer address) -> i32;

        auto read_i64(pointer address) -> i64;

        auto read_u8(pointer address) -> u8;

        auto read_u16(pointer address) -> u16;

        auto read_u32(pointer address) -> u32;

        auto read_u64(pointer address) -> u64;

        auto read_float(pointer address) -> float;

        auto read_double(pointer address) -> double;

        auto read_bytes(pointer address, size_t size) -> std::vector<char>;

        auto read_string(pointer address, size_t size) -> std::string;

        auto read_pointer(pointer address) -> pointer;
    };
}

#endif // READER_H