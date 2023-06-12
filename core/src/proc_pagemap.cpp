#include "proc_pagemap.h"

#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include "memory.h"

namespace hak {
    auto get_pagemap_entry(handle pagemap_fd, pointer vaddr) -> pagemap_entry { // NOLINT(*-easily-swappable-parameters)
        pagemap_entry entry;
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
}