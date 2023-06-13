#include "reader.h"

hak::memory_reader::memory_reader(std::shared_ptr<hak::process> proc) {
    this->process = proc;
}

auto hak::memory_reader::read_i8(pointer address) -> i8 {
    return read<i8>(address);
}

auto hak::memory_reader::read_i16(pointer address) -> i16 {
    return read<i16>(address);
}

auto hak::memory_reader::read_i32(pointer address) -> i32 {
    return read<i32>(address);
}

auto hak::memory_reader::read_i64(pointer address) -> i64 {
    return read<i64>(address);
}

auto hak::memory_reader::read_u8(pointer address) -> u8 {
    return read<u8>(address);
}

auto hak::memory_reader::read_u16(pointer address) -> u16 {
    return read<u16>(address);
}

auto hak::memory_reader::read_u32(pointer address) -> u32 {
    return read<u32>(address);
}

auto hak::memory_reader::read_u64(pointer address) -> u64 {
    return read<u64>(address);
}

auto hak::memory_reader::read_float(pointer address) -> float {
    return read<float>(address);
}

auto hak::memory_reader::read_double(pointer address) -> double {
    return read<double>(address);
}

auto hak::memory_reader::read_bytes(pointer address, size_t size) -> std::vector<char> {
    char temp[size];
    this->process->read(address, &temp, size);
    std::vector<char> vec(size);
    vec.insert(vec.end(), temp, temp + size);
    return std::move(vec);
}

auto hak::memory_reader::read_pointer(pointer address) -> pointer {
    return read<pointer>(address);
}

auto hak::memory_reader::read_string(pointer address, size_t size) -> std::string {
    char temp[size];
    this->process->read(address, &temp, size);
    return {temp};
}

void hak::memory_reader::read(pointer address, void *data, size_t size) {
    this->process->read(address, data, size);
}

template<typename T>
auto hak::memory_reader::read(pointer address) -> T {
    T temp;
    this->process->read(address, &temp, sizeof(T));
    return temp;
}
