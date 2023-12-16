#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-easily-swappable-parameters"
#include "memory.h"
#include "exception.h"

#include <fstream>
#include <sstream>
#include <vector>

void hak::read_by_syscall(pid_t pid, pointer addr, void *data, size_t len) {
    iovec local{ data, len };
    iovec remote{ (void*) addr, len };
    if (process_vm_readv(pid, &local, 1, &remote, 1, 0) != len) {
        std::string hexStr = (std::stringstream() << "Memory read Error: " << std::hex << addr << ".").str();
        throw hak::memory_operate_error(hexStr);
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

std::vector<hak::proc_maps> hak::get_process_maps(pid_t pid) {
    std::ifstream maps(std::string("/proc/") + std::to_string(pid) + "/maps");
    if (!maps.is_open()) {
        throw hak::file_not_found();
    }

    std::vector<hak::proc_maps> result;

    std::string line;
    bool last_is_cd = false;
    while (getline(maps, line)) {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;

        while (getline(iss, token, ' ')) {
            tokens.push_back(token);
        }

        auto address = tokens[0];
        std::string::size_type pos = address.find('-');
        pointer start_addr = std::stol(address.substr(0, pos), nullptr, 16);
        pointer end_addr = std::stol(address.substr(pos + 1), nullptr, 16);
        auto pmaps = hak::proc_maps(start_addr, end_addr);
        auto perms = tokens[1];
        pmaps.readable = perms[0] == 'r';
        pmaps.writable = perms[1] == 'w';
        pmaps.executable = perms[2] == 'x';
        pmaps.is_private = perms[3] == 'p';
        pmaps.offset = std::stoll(tokens[2], nullptr, 16);
        pmaps.inode = std::stoi(tokens[4]);
        std::string module_name;
        if (tokens.size() > 5) {
            for (int i = 5; i < tokens.size(); i++) {
                module_name += tokens[i];
            }
        }
        if (module_name.size() < 128) {
            module_name.copy(pmaps.module_name, module_name.size());
            pmaps.module_name[module_name.size()] = '\0';
        }
        determine_range(&pmaps, last_is_cd);
        last_is_cd = pmaps.range == hak::memory_range::CD;

        result.push_back(pmaps);
    }
    maps.close();
    return result;
}

std::shared_ptr<hak::proc_maps> hak::get_process_map(pid_t pid, std::string &name) {
    auto maps = get_process_maps(pid);
    for (auto &item : maps) {
        if (strstr(item.module_name, name.c_str())) {
            return std::make_shared<hak::proc_maps>(item);
        }
    }
    return nullptr;
}

#pragma clang diagnostic pop