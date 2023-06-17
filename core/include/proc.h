#ifndef PROC_H
#define PROC_H

#include <sys/uio.h>
#include <filesystem>
#include <dirent.h>
#include <string>
#include <vector>

#include "types.h"

namespace hak {
    auto get_pid_list() -> std::vector<pid_t>;

    auto find_process(std::string& package) -> pid_t;
}

#endif // PROC_H