#include "fuzzy.h"

#include <utility>

#include "exception.h"

hak::memory_fuzzy::memory_fuzzy(std::shared_ptr<hak::process> proc) : memory_searcher(std::move(proc)) {

}

void hak::memory_fuzzy::set_memory_range(i32 _range) {
    if (_range == ALL) {
        throw memory_operate_error("memory_fuzzy not support a specific range");
    }
    memory_searcher::set_memory_range(_range);
}

void hak::memory_fuzzy::load_memory_data() {
    if (range == ALL) {
        throw memory_operate_error("memory_fuzzy not support a specific range");
    }
    std::vector<std::pair<pointer, pointer>> pages;
    memory_searcher::organize_memory_page_groups(pages);


    std::for_each(pages.begin(), pages.end(), [&](const std::pair<pointer, pointer> &item) {
        auto start = item.first;
        auto end = item.second;
        auto value_size = memory_searcher::get_value_size_by_type(type);
        pointer addr = start;
        do {

            addr += value_size; // NOLINT(*-narrowing-conversions)
        } while (addr < end && (end - addr) >= value_size);
    });
}



