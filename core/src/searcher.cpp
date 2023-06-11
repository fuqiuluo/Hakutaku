#include "searcher.h"

#include <any>
#include <vector>
#include <iostream>

hak::memory_searcher::memory_searcher(std::shared_ptr<hak::process> process) {
    this->process = process;
}

void hak::memory_searcher::searchNumber(const std::string &expr, value_type default_type) {
    auto values = hak::parse_search_number_expr(expr, default_type);

    std::for_each(values.begin(), values.end(), [](hak::value& item) {
        if (item.index() == type_i32) {
            std::cout << "int: " << std::get<i32>(item) << "\n";
        }
    });
}
