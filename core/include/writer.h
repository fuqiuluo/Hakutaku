#ifndef WRITER_H
#define WRITER_H

#include "types.h"
#include "process.h"

#include <vector>
#include <string>

namespace hak {
    class memory_writer {
        std::shared_ptr<hak::process> process;
    public:
        explicit memory_writer(std::shared_ptr<hak::process> proc);

        void write(pointer address, void* data, size_t size);

        template<typename T>
        void write(pointer address, T data);

        void write_i8(pointer address, i8 data);

        void write_i16(pointer address, i16 data);

        void write_i32(pointer address, i32 data);

        void write_i64(pointer address, i64 data);

        void write_u8(pointer address, u8 data);

        void write_u16(pointer address, u16 data);

        void write_u32(pointer address, u32 data);

        void write_u64(pointer address, u64 data);

        void write_float(pointer address, float data);

        void write_double(pointer address, double data);

        void write_bytes(pointer address, char* data, size_t size);
        void write_bytes(pointer address, std::vector<char>& data);
        void write_bytes(pointer address, std::string& data);
    };
}

#endif // WRITER_H