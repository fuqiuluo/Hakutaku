#ifndef PROC_H
#define PROC_H

#include <sys/uio.h>
#include <filesystem>
#include <dirent.h>
#include <string>
#include <vector>

#include "types.h"

namespace hak {
    class proc_stat {
    public:
        pid_t pid;
        std::string comm;
        char state;
        pid_t ppid;
    };

    auto get_process_list() -> std::vector<hak::proc_stat>;

    auto get_pid_list() -> std::vector<pid_t>;

    auto find_process(std::string& package) -> pid_t;
}

#endif // PROC_H