#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

#include <memory>
#include <cstring>
#include <sys/uio.h>
#include <unistd.h>

namespace hak {
    void read_direct(pointer addr, void *data, size_t len);

    void write_direct(pointer addr, void *data, size_t len);

    void read_by_mem(int mem_fd, pointer addr, void *data, size_t len);

    void write_by_mem(int mem_fd, pointer addr, void *data, size_t len);

    void read_by_syscall(pid_t pid, pointer addr, void *data, size_t len);

    void write_by_syscall(pid_t pid, pointer addr, void *data, size_t len);
}

#endif // MEMORY_H