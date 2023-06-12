#ifndef HAK_PROC_PAGEMAP_H
#define HAK_PROC_PAGEMAP_H

#include "types.h"

namespace hak {
    class pagemap_entry {
    public:
        u64 pfn : 54;
        u32 soft_dirty : 1;
        u32 file_page : 1;
        u32 swapped : 1;
        u32 present : 1;
    };

    auto get_pagemap_entry(handle pagemap_fd, pointer vaddr) -> pagemap_entry;
}

#endif //HAK_PROC_PAGEMAP_H
