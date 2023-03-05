#include "Hakutaku.h"

#include <dirent.h>

namespace Hakutaku {
    Pid getPid(std::string& packageName) {
        auto proc_dir = opendir(PLAT_PROC_DIR);
        if (proc_dir == nullptr) {
            return 0;
        }
        FILE* fp;
        std::string file_path;
        dirent* pid_file;
        char cmd_line[128];

        while ((pid_file = readdir(proc_dir))) {
            if (pid_file->d_type != DT_DIR ||
            strcmp(pid_file->d_name, ".") == 0 ||
            strcmp(pid_file->d_name, "..") == 0)
                continue;

            file_path = "/proc/";
            file_path += pid_file->d_name;
            file_path += "/cmdline";

            fp = fopen(file_path.c_str(), "r");
            if (fp != nullptr) {
                std::fgets(cmd_line, sizeof(cmd_line), fp);
                std::fclose(fp);
                if (packageName == cmd_line) {
                    closedir(proc_dir);
                    return (int) std::strtol(pid_file->d_name, nullptr, 0);
                }
            }
        }
        closedir(proc_dir);
        return 0;
    }
}