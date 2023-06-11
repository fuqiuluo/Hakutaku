#ifndef PROC_MAPS_H
#define PROC_MAPS_H

#include "types.h"

#include <memory>

namespace hak {
    enum memory_range {
        ALL = 65534,
        BAD = 2,
        V = 4, // video
        CA = 8,
        CB = 16,
        CD = 32,
        CH = 64,
        JH = 128, // Java heap
        J = 256, // Java
        A = 512,
        XS = 1024, // 系统空间代码
        S = 2048, // 栈区
        AS = 4096,
        OTHER = 8192,
        XA = 8192 * 2, // 用户空间代码
        PS = 8192 * 2 * 2
    };

    class proc_maps: public std::enable_shared_from_this<proc_maps> {
        std::shared_ptr<proc_maps> _head;
        std::shared_ptr<proc_maps> _tail;

        pointer _start;
        pointer _end;
    public:
        proc_maps(pointer start, pointer end);

        memory_range range = ALL;
        bool readable = false;
        bool writable = false;
        bool executable = false;
        bool is_private = false;
        u32 inode = -1;
        i64 offset = 0;
        char module_name[128];

        void insert(std::shared_ptr<proc_maps> maps);

        void remove();

        // The speed of this api is not very fast~~~!
        auto size() -> size_t;

        [[nodiscard]] auto start() const -> pointer;

        [[nodiscard]] auto end() const -> pointer;

        auto last() -> std::shared_ptr<proc_maps>;

        auto next() -> std::shared_ptr<proc_maps>&;
    };

    auto get_maps(pid_t pid, i32 range = memory_range::ALL) -> std::shared_ptr<proc_maps>;
}

#endif // PROC_MAPS_H