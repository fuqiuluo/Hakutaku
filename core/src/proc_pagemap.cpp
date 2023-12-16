#include "proc_pagemap.h"

#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include "memory.h"

namespace hak {
    auto get_pagemap_entry(handle pagemap_fd, pointer vaddr) -> pagemap_entry { // NOLINT(*-easily-swappable-parameters)
        pagemap_entry entry{};
        size_t nread;
        int64_t ret;
        uint64_t data;
        pointer file_offset = (vaddr / sysconf(_SC_PAGE_SIZE)) * sizeof(data); // NOLINT(*-narrowing-conversions)
        struct iovec iov{ &data, sizeof(data) };
        nread = 0;
        while (nread < sizeof(data)) {
            iov.iov_len = sizeof(data) - nread;
            ret = preadv(pagemap_fd, &iov, 1, file_offset);
            nread += ret;
            if (ret <= 0) {
                return entry;
            }
        }
        entry.pfn = data & (((uint64_t)1 << 54) - 1);
        entry.soft_dirty = (data >> 54) & 1;
        entry.file_page = (data >> 61) & 1;
        entry.swapped = (data >> 62) & 1;
        entry.present = (data >> 63) & 1;
        return entry;
    }

    auto virt_to_phy(pid_t pid, pointer virAddr) -> pointer { // NOLINT(*-easily-swappable-parameters)
        char path[30] = {0};
        sprintf(path , "/proc/%d/pagemap", pid);
        int fd = open(path, O_RDONLY);
        if(fd < 0) {
            printf("open '/proc/self/pagemap' failed!\n");
            return 0;
        }
        size_t pagesize = getpagesize();
        size_t offset = (virAddr / pagesize) * sizeof(uint64_t);
        if(lseek(fd, (long) offset, SEEK_SET) < 0) {
            printf("lseek() failed!\n");
            close(fd);
            return 0 ;
        }
        uint64_t info;
        if(read(fd, &info, sizeof(uint64_t)) != sizeof(uint64_t)) {
            printf("read() failed!\n");
            close(fd);
            return 0;
        }
        if((info & (((uint64_t)1 << 63))) == 0) {
            printf("page is not present!\n");
            close(fd);
            return 0;
        }
        size_t pageframenum = info & (((uint64_t)1 << 55) -1);
        size_t phyaddr = pageframenum * pagesize + virAddr % pagesize;
        close(fd);
        return phyaddr; // NOLINT(*-narrowing-conversions)
    }
}