#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-easily-swappable-parameters"
#include "memory.h"
#include "exception.h"

void hak::read_by_syscall(pid_t pid, pointer addr, void *data, size_t len) {
    iovec local{ data, len };
    iovec remote{ (void*) addr, len };
    if (process_vm_readv(pid, &local, 1, &remote, 1, 0) != len) {
        throw hak::memory_operate_error("Memory read error.");
    }
}

void hak::write_by_syscall(pid_t pid, pointer addr, void *data, size_t len) {
    iovec local{ data, len };
    iovec remote{ (void*) addr, len };
    if (process_vm_writev(pid, &local, 1, &remote, 1, 0) != len) {
        throw hak::memory_operate_error("Memory write error.");
    }
}

void hak::read_direct(pointer addr, void *data, size_t len) {
    memcpy(data, (void*) addr, len);
}

void hak::write_direct(pointer addr, void *data, size_t len) {
    memcpy((void*) addr, data, len);
}

void hak::read_by_mem(int mem_fd, pointer addr, void *data, size_t len) {
    pread64(mem_fd, data, len, addr);
}

void hak::write_by_mem(int mem_fd, pointer addr, void *data, size_t len) {
    pwrite64(mem_fd, data, len, addr);
}

#pragma clang diagnostic pop