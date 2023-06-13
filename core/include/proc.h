#ifndef PROC_H
#define PROC_H

#include <sys/uio.h>
#include <filesystem>
#include <dirent.h>
#include <string>

#include "types.h"

namespace hak {
    auto find_process(std::string& package) -> pid_t;
}

#endif // PROC_H