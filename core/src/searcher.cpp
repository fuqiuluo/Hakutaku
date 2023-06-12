#include "searcher.h"

#include <any>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>

namespace hak {
    memory_searcher::memory_searcher(std::shared_ptr<hak::process> process) {
        this->process = process;
    }

    void memory_searcher::set_memory_range(i32 range) {
        this->range = range;
    }

    void memory_searcher::searchNumber(const std::string &expr, value_type default_type) {
        auto values = hak::parse_search_number_expr(expr, default_type);
        auto maps = process->get_maps(this->range);
        do {
            auto start = maps->start();
            auto end = maps->end();

        } while ((maps = maps->next()));
    }


}



