#include "proc.h"
#include "exception.h"

#include <string>
#include <istream>
#include <sstream>
#include <cctype>

template <size_t N>
auto read_proc_file(pid_t pid, const char *name, char *dest) -> void {
    std::string file_path = "/proc/" + std::to_string(pid) + "/" + name;
    FILE* _fp = fopen(file_path.c_str(), "r");
    if (_fp != nullptr) {
        std::fgets(dest, N, _fp);
        std::fclose(_fp);
    } else {
        dest[0] = '\0';
    }
}

auto hak::get_process_list() -> std::vector<hak::proc_stat> {
    std::vector<hak::proc_stat> list;
    auto *proc_dir = opendir("/proc");
    if (proc_dir == nullptr) {
        throw std::runtime_error("Failed to open /proc.");
    }
    struct dirent* pid_file;
    char stat[256];
    while ((pid_file = readdir(proc_dir)) != nullptr) {
        if (pid_file->d_type != DT_DIR || ((std::isdigit(pid_file->d_name[0])) == 0)) {
            continue;
        }
        hak::proc_stat my_stat;
        pid_t pid = std::stoi(pid_file->d_name);
        read_proc_file<sizeof(stat)>(pid, "stat", stat);
        std::istringstream iss((std::string(stat)));
        iss >> my_stat.pid;
        //my_stat.pid = pid;
        iss >> my_stat.comm;
        iss >> my_stat.state;
        iss >> my_stat.ppid;
        if (my_stat.state == 'R' || my_stat.state == 'S' || my_stat.state == 'D') {
            list.push_back(my_stat);
        }
    }
    closedir(proc_dir);
    return std::move(list);
}

auto hak::get_pid_list() -> std::vector<pid_t> {
    std::vector<pid_t> list;
    auto *proc_dir = opendir("/proc");
    if (proc_dir == nullptr) {
        throw std::runtime_error("Failed to open /proc.");
    }
    struct dirent* pid_file;
    char stat[256];
    while ((pid_file = readdir(proc_dir)) != nullptr) {
        if (pid_file->d_type != DT_DIR || ((std::isdigit(pid_file->d_name[0])) == 0)) {
            continue;
        }
        pid_t pid = std::stoi(pid_file->d_name);
        read_proc_file<sizeof(stat)>(pid, "stat", stat);
        std::istringstream iss((std::string(stat)));
        std::string token;
        for (int i = 0; i < 3; ++i) {
            iss >> token;
        }
        if (token == "R" || token == "S" || token == "D") {
            list.emplace_back(pid);
        }
    }
    closedir(proc_dir);
    return std::move(list);
}

auto hak::find_process(std::string &package) -> pid_t {
    auto *proc_dir = opendir("/proc");
    if (proc_dir == nullptr) {
        throw std::runtime_error("Failed to open /proc.");
    }
    struct dirent* pid_file;
    char cmd_line[128];
    while ((pid_file = readdir(proc_dir)) != nullptr) {
        if (pid_file->d_type != DT_DIR || ((std::isdigit(pid_file->d_name[0])) == 0)) {
            continue;
        }
        pid_t pid = std::stoi(pid_file->d_name);
        read_proc_file<sizeof(cmd_line)>(pid, "cmdline", cmd_line);
        if (package == cmd_line) {
            closedir(proc_dir);
            return std::stoi(pid_file->d_name);
        }
    }
    closedir(proc_dir);
    throw no_process_error();
}


