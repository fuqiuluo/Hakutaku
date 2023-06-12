#include <fcntl.h>
#include <string>
#include <iostream>
#include <sys/mman.h>
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

auto hak::process::is_missing_page(pointer address) -> bool {
    if (this->mem_mode == DIRECT) {
        auto pagesize = getpagesize();
        unsigned char vec = 0;
        mincore((void*) (address & (~(pagesize - 1))), pagesize, &vec);
        return vec == 1;
    }
    this->init_pagemaps_fd();
    size_t nread;
    int64_t ret;
    uint64_t data;
    pointer file_offset = (address / sysconf(_SC_PAGE_SIZE)) * sizeof(data); // NOLINT(*-narrowing-conversions)
    struct iovec iov{ &data, sizeof(data) };
    nread = 0;
    while (nread < sizeof(data)) {
        iov.iov_len = sizeof(data) - nread;
        ret = preadv(this->pagemap_fd, &iov, 1, file_offset);
        nread += ret;
        if (ret <= 0) {
            return false;
        }
    }
    //auto pfn = data & (((uint64_t)1 << 54) - 1);
    //auto soft_dirty = (data >> 54) & 1;
    //auto file_page = (data >> 61) & 1;
    //auto swapped = (data >> 62) & 1;
    auto present = (data >> 63) & 1;
    //return present == 0 && swapped == 0;
    return present == 0;
}
