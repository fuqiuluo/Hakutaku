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
    protected:
        std::shared_ptr<hak::process> process;
        i32 range = ALL;

        struct {
            bool ignore_swapped_page: 1 = false;
            bool ignore_missing_page: 1 = true;

            pointer start: 8 = 0;
            pointer end: 8 = 0;
        } config;
        //std::vector<pointer> results;
        std::unordered_set<pointer> results;
    public:
        explicit memory_searcher(std::shared_ptr<hak::process> process);

        virtual void set_memory_range(i32 _range);

        void set_ignore_swapped_page(bool ignore);

        void set_ignore_missing_page(bool ignore);

        void set_search_range(pointer start, pointer end);

        void clear_results();

        auto get_results() -> const std::unordered_set<pointer>&;

        auto searchNumber(const std::string& expr, value_type default_type, match_sign sign = EQ) -> size_t;

        auto filterNumber(const std::string& address, value_type default_type, match_sign sign = EQ) -> size_t;

        static auto get_value_size(value value) -> size_t;

        static auto get_value_size_by_type(value_type type) -> size_t;
    protected:
        void organize_memory_page_groups(std::vector<std::pair<pointer, pointer>>& dest);
    };
}

#endif