#ifndef PROC_MAPS_H
#define PROC_MAPS_H

#include "types.h"

#include <memory>

namespace hak {
    enum memory_range {
        ALL =   0b1111111111111110,
       //CP =   0b0000000000000001,
        BAD =   0b0000000000000010,
        V =     0b0000000000000100, // video
        CA =    0b0000000000001000,
        CB =    0b0000000000010000,
        CD =    0b0000000000100000,
        CH =    0b0000000001000000,
        JH =    0b0000000010000000, // Java heap
        J =     0b0000000100000000, // Java
        A =     0b0000001000000000,
        XS =    0b0000010000000000, // 系统空间代码
        S =     0b0000100000000000, // 栈区
        AS =    0b0001000000000000,
        OTHER = 0b0010000000000000,
        XA =    0b0100000000000000, // 用户空间代码
        PS =    0b1000000000000000
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

    void determine_range(hak::proc_maps* maps, bool last_is_cd);
}

#endif // PROC_MAPS_H