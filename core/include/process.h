#ifndef PROCESS_H
#define PROCESS_H

#include <sys/uio.h>
#include <memory>

#include "proc_maps.h"
#include "types.h"

namespace hak {
    enum work_mode {
        ROOT,
        NON_ROOT
    };

    enum memory_mode {
        DIRECT, MEM_FILE, SYSCALL,
        PTRACE // not supported
    };

    class process: public std::enable_shared_from_this<process> {
        pid_t pid;
        memory_mode mem_mode;

        handle mem_fd;
    public:
        explicit process(pid_t pid);
        ~process();

        [[nodiscard]] auto get_maps(i32 range = ALL) const -> std::shared_ptr<proc_maps>;

        void read(pointer addr, void *data, size_t len);

        void write(pointer addr, void *data, size_t len);

        void set_memory_mode(memory_mode mode) {
            this->mem_mode = mode;
        }
    private:
        void init_mem_fd();
    };
}

#endif // PROCESS_H