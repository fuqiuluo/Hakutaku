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
    public:
        explicit memory_searcher(std::shared_ptr<hak::process> process);

        void searchNumber(const std::string& expr, value_type default_type);
    };
}

#endif