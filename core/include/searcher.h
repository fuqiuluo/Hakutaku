#ifndef SEARCHER_H
#define SEARCHER_H

#include "types.h"
#include "process.h"
#include "searcher_parser.h"

#include <memory>
#include <string>

namespace hak {
    class memory_searcher {
        std::shared_ptr<hak::process> process;
        i32 range = ALL;
    public:
        explicit memory_searcher(std::shared_ptr<hak::process> process);

        void set_memory_range(i32 range);

        void searchNumber(const std::string& expr, value_type default_type);
    };
}

#endif