#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"
#include "proc_maps.h"

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

    std::vector<hak::proc_maps> get_process_maps(pid_t pid);

    std::shared_ptr<hak::proc_maps> get_process_map(pid_t pid, std::string &name);
}

#endif // MEMORY_H