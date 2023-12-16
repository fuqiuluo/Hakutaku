#include <fcntl.h>
#include <string>
#include <sstream>
#include "process.h"
#include "memory.h"

auto hak::process::get_maps(i32 range) const -> std::shared_ptr<hak::proc_maps> {
    return hak::get_maps(this->pid, range);
}

hak::process::process(pid_t pid) {
    this->pid = pid;
    this->mem_mode = hak::memory_mode::SYSCALL;
    this->mem_fd = 0;
    this->pagemap_fd = 0;
}

void hak::process::read(pointer addr, void *data, size_t len) {
    auto entry = get_page_entry(addr);
    if (!entry.present) {
        throw std::runtime_error("The page is not present.");
    }
    if (this->mem_mode == DIRECT) {
        read_direct(addr, data, len);
    } else if (this->mem_mode == MEM_FILE) {
        init_mem_fd();
        read_by_mem(this->mem_fd, addr, data, len);
    } else if (this->mem_mode == SYSCALL) {
        read_by_syscall(this->pid, addr, data, len);
    } else {
        throw std::runtime_error("Not support the mem_mode.");
    }
}

void hak::process::write(pointer addr, void *data, size_t len) {
    if (this->mem_mode == DIRECT) {
        write_direct(addr, data, len);
    } else if (this->mem_mode == MEM_FILE) {
        init_mem_fd();
        write_by_mem(this->mem_fd, addr, data, len);
    } else if (this->mem_mode == SYSCALL) {
        write_by_syscall(this->pid, addr, data, len);
    } else {
        throw std::runtime_error("Not support the mem_mode.");
    }
}

void hak::process::init_mem_fd() {
    if (this->mem_fd == 0) {
        std::string path = "/proc/" + std::to_string(pid) + "/mem";
        this->mem_fd = open(path.c_str(), O_RDWR);
    }
}

hak::process::~process() {
    if (this->mem_fd != 0) {
        close(this->mem_fd);
        this->mem_fd = 0;
    }
    if (this->pagemap_fd != 0) {
        close(this->pagemap_fd);
        this->pagemap_fd = 0;
    }
}

void hak::process::set_memory_mode(hak::memory_mode mode) {
    this->mem_mode = mode;
}

void hak::process::init_pagemaps_fd() {
    if (this->pagemap_fd == 0) {
        auto pid_str = std::to_string(this->pid);
        std::string path = "/proc/" + pid_str + "/task/" + pid_str + "/pagemap";
        this->pagemap_fd = open(path.c_str(), O_RDONLY);
        if (this->pagemap_fd == 0) {
            throw std::runtime_error("Open /proc/pid/pagemap error.");
        }
    }
}

auto hak::process::get_page_entry(pointer address) -> hak::pagemap_entry {
    this->init_pagemaps_fd();
    return hak::get_pagemap_entry(this->pagemap_fd, address);
}

auto hak::process::is_running() const -> bool {
    char stat[256];
    std::string file_path = "/proc/" + std::to_string(this->pid) + "/stat";
    FILE* _fp = fopen(file_path.c_str(), "r");
    if (_fp != nullptr) {
        std::fgets(stat, sizeof(stat), _fp);
        std::fclose(_fp);
        std::istringstream iss((std::string(stat)));
        std::string token;
        for (int i = 0; i < 3; ++i) {
            iss >> token;
        }
        if (token == "R" || token == "S" || token == "D") {
            return true;
        }
    }
    return false;
}

void hak::process::read_pointer(pointer addr, pointer *data) {
    read(addr, data, sizeof(pointer));
}

void hak::process::dump_memory(pointer addr, int line) {
    char data[16];
    for (int i = 0; i < line; ++i) {
        read(addr + i * 16, data, sizeof(data));
        printf("%p | ", (void*) (addr + i * 16));
        for (char j : data) {
            printf("%02x ", (unsigned char) j);
        }
        printf("\n");
    }
}
