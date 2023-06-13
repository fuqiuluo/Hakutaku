#ifndef SEARCHER_H
#define SEARCHER_H

#include "types.h"
#include "process.h"
#include "searcher_parser.h"

#include <memory>
#include <string>
#include <unordered_set>

namespace hak {
    enum match_sign {
        EQ, // ==
        NE, // !=
        GT, // >
        GE, // >=
        LT, // <
        LE, // <=
    };

    class memory_searcher {
        std::shared_ptr<hak::process> process;
        i32 range = ALL;

        bool ignore_swapped_page = true;
        bool ignore_missing_page = true;

        //std::vector<pointer> results;
        std::unordered_set<pointer> results;
    public:
        explicit memory_searcher(std::shared_ptr<hak::process> process);

        void set_memory_range(i32 _range);

        void set_ignore_swapped_page(bool ignore);

        void set_ignore_missing_page(bool ignore);

        auto searchNumber(const std::string& callback_addr, value_type default_type, match_sign sign = EQ) -> size_t;
    };
}

#endif