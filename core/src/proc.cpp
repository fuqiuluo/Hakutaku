#include "proc.h"
#include "exception.h"

#include <string>
#include <cctype>

auto hak::find_process(std::string &package) -> pid_t {
    /*
    // As a modern c++, the efficiency is not flattering.
    pid_t r_pid = -1;
    for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
        if (entry.is_regular_file() && entry.path().filename() == "cmdline") {
            std::ifstream cmdline_file(entry.path());
            if (cmdline_file) {
                std::string cmdline_content((std::istreambuf_iterator<char>(cmdline_file)), std::istreambuf_iterator<char>());
                if (cmdline_content == package) {
                    r_pid = std::stoi(entry.path().parent_path().filename());
                    break;
                }
            }
        }
    }
    if (r_pid == -1) {
        throw no_process_error();
    }*/
    auto *proc_dir = opendir("/proc");
    if (proc_dir == nullptr) {
        throw std::runtime_error("Failed to open /proc.");
    }
    FILE* _fp;
    struct dirent* pid_file;
    char cmd_line[128];
    // char file_path[128];
    while ((pid_file = readdir(proc_dir)) != nullptr) {
        if (pid_file->d_type != DT_DIR || ((std::isdigit(pid_file->d_name[0])) == 0)) {
            continue;
        }
        // file_path.erase(6, file_path.size() - 6); // slowly~~~!
        std::string file_path = "/proc/";
        file_path += pid_file->d_name;
        file_path += "/cmdline";
        // sprintf(file_path, "/proc/%s/cmdline", pid_file->d_name); // slowly~~~!
        _fp = fopen(file_path.c_str(), "r");
        if (_fp != nullptr) {
            std::fgets(cmd_line, sizeof(cmd_line), _fp);
            std::fclose(_fp);
            if (package == cmd_line) {
                closedir(proc_dir);
                return std::stoi(pid_file->d_name);
            }
        }
    }
    closedir(proc_dir);
    throw no_process_error();
}