#include "writer.h"

hak::memory_writer::memory_writer(std::shared_ptr<hak::process> proc) {
    this->process = proc;
}

void hak::memory_writer::write(pointer address, void *data, size_t size) {
    this->process->write(address, data, size);
}

void hak::memory_writer::write_i8(pointer address, i8 data) {
    write(address, data);
}

void hak::memory_writer::write_i16(pointer address, i16 data) {
    write(address, data);
}

void hak::memory_writer::write_i32(pointer address, i32 data) {
    write(address, data);
}

void hak::memory_writer::write_i64(pointer address, i64 data) {
    write(address, data);
}

void hak::memory_writer::write_u8(pointer address, u8 data) {
    write(address, data);
}

void hak::memory_writer::write_u16(pointer address, u16 data) {
    write(address, data);
}

void hak::memory_writer::write_u32(pointer address, u32 data) {
    write(address, data);
}

void hak::memory_writer::write_u64(pointer address, u64 data) {
    write(address, data);
}

void hak::memory_writer::write_float(pointer address, float data) {
    write(address, data);
}

void hak::memory_writer::write_double(pointer address, double data) {
    write(address, data);
}

void hak::memory_writer::write_bytes(pointer address, char *data, size_t size) {
    write(address, data, size);
}

void hak::memory_writer::write_bytes(pointer address, std::vector<char> &data) {
    write(address, data.data(), data.size());
}

void hak::memory_writer::write_bytes(pointer address, std::string &data) {
    write(address, (void *) data.c_str(), data.length());
}

template<typename T>
void hak::memory_writer::write(pointer address, T data) {
    this->process->write(address, &data, sizeof(T));
}
