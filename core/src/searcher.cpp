#include "searcher.h"

#include <any>
#include <vector>
#include <functional>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>

namespace hak {
    memory_searcher::memory_searcher(std::shared_ptr<hak::process> process) {
        this->process = process;
    }

    void memory_searcher::set_memory_range(i32 _range) {
        this->range = _range;
    }

    void memory_searcher::set_ignore_swapped_page(bool ignore) {
        this->ignore_swapped_page = ignore;
    }

    void memory_searcher::set_ignore_missing_page(bool ignore) {
        this->ignore_missing_page = ignore;
    }

    auto get_legal_pages(std::shared_ptr<process>& process, std::shared_ptr<proc_maps>& maps,
                         bool ignore_swapped_page, bool ignore_missing_page, std::vector<std::pair<pointer, pointer>>& dest) -> void {
        if (!maps->readable) {
            return;
        }
        auto page_size = getpagesize();
        for (int i = 0; i < ((maps->end() - maps->start()) / page_size); ++i) {
            auto start = maps->start() + (i * page_size);
            auto entry = process->get_page_entry(start);
            if (!entry.present && !ignore_missing_page) {
                continue;
            }
            if (!entry.swapped && !ignore_swapped_page) {
                continue;
            }
            auto end = start + page_size;
            dest.emplace_back(start, end);
        }
    }

    auto organize_memory_page_groups(std::shared_ptr<process>& process,
                                     bool ignore_swapped_page, bool ignore_missing_page, i32 range, std::vector<std::pair<pointer, pointer>>& dest) {
        std::vector<std::pair<pointer, pointer>> pages;
        auto maps = process->get_maps(range);
        do {
            get_legal_pages(process, maps, ignore_swapped_page, ignore_missing_page, pages);
        } while ((maps = maps->next()));

        pointer start;
        pointer end;
        std::for_each(pages.begin(), pages.end(), [&](const std::pair<pointer, pointer> &item) {
            if (start == 0) {
                start = item.first;
                end = item.second;
            } else {
                if (end == item.first) {
                    end = item.second;
                } else {
                    dest.emplace_back(start, end);
                    start = item.first;
                    end = item.second;
                }
            }
        });
    }

    template<typename T>
    auto match_number_value(std::shared_ptr<process>& process, pointer address, T& value, match_sign sign) -> bool {
        T temp;
        process->read(address, &temp, sizeof(temp));
        if (sign == EQ && temp == value) {
            return true;
        }
        if (sign == NE && temp != value) {
            return true;
        }
        if (sign == GT && temp > value) {
            return true;
        }
        if (sign == GE && temp >= value) {
            return true;
        }
        if (sign == LT && temp < value) {
            return true;
        }
        if (sign == LE && temp <= value) {
            return true;
        }
        return false;
    }

    template<typename T>
    auto match_number_value(std::shared_ptr<process>& process, pointer address, hak::value& value, match_sign sign) -> bool {
        auto val = std::get<T>(value);
        return match_number_value(process, address, val, sign);
    }

    template<typename T>
    auto match_range_value(std::shared_ptr<process>& process, pointer address, hak::value& value, match_sign l_sign, match_sign r_sign) {
        auto range = std::get<hak::range>(value);
        auto start = std::get<i8>(range.start);
        auto end = std::get<i8>(range.end);
        return match_number_value(process, address, start, l_sign) && match_number_value(process, address, end, r_sign);
    }

    auto match_range_value(std::shared_ptr<process>& process, pointer address, hak::value& value, match_sign sign) -> bool {
        auto range = std::get<hak::range>(value);
        switch (range.type) {
            case type_i8: {
                return match_range_value<i8>(process, address, value, GE, LE);
            }
            case type_i16: {
                return match_range_value<i16>(process, address, value, GE, LE);
            }
            case type_i32: {
                return match_range_value<i32>(process, address, value, GE, LE);
            }
            case type_i64: {
                return match_range_value<i64>(process, address, value, GE, LE);
            }
            case type_u8: {
                return match_range_value<u8>(process, address, value, GE, LE);
            }
            case type_u16: {
                return match_range_value<u16>(process, address, value, GE, LE);
            }
            case type_u32: {
                return match_range_value<u32>(process, address, value, GE, LE);
            }
            case type_u64: {
                return match_range_value<u64>(process, address, value, GE, LE);
            }
            case type_float: {
                return match_range_value<float>(process, address, value, GE, LE);
            }
            case type_double: {
                return match_range_value<double>(process, address, value, GE, LE);
            }
            case type_range:
            case type_unknown:
                break;
        }
        return false;
    }

    auto match_value(std::shared_ptr<process>& process, hak::value& value, pointer address, match_sign sign) -> bool {
        switch (value.index()) {
            case type_i8: {
                return match_number_value<i8>(process, address, value, sign);
            }
            case type_i16: {
                return match_number_value<i16>(process, address, value, sign);
            }
            case type_i32: {
                return match_number_value<i32>(process, address, value, sign);
            }
            case type_i64: {
                return match_number_value<i64>(process, address, value, sign);
            }
            case type_u8: {
                return match_number_value<u8>(process, address, value, sign);
            }
            case type_u16: {
                return match_number_value<u16>(process, address, value, sign);
            }
            case type_u32: {
                return match_number_value<u32>(process, address, value, sign);
            }
            case type_u64: {
                return match_number_value<u64>(process, address, value, sign);
            }
            case type_float: {
                return match_number_value<float>(process, address, value, sign);
            }
            case type_double: {
                return match_number_value<double>(process, address, value, sign);
            }
            case type_range: {
                return match_range_value(process, address, value, sign);
            }
        }
        return false;
    }

    void scan_value(std::shared_ptr<process>& process, std::pair<pointer, pointer>& pair, std::vector<pointer>& result) {

    }

    void walk_matched_value(std::shared_ptr<process>& process, pointer start, pointer end, std::vector<value>::iterator value) {
        switch (value->index()) {
            case type_i8: {

            }

        }
    }

    void memory_searcher::searchNumber(const std::string &expr, value_type default_type, match_sign sign) {
        auto values = hak::parse_search_number_expr(expr, default_type);
        if (values.empty()) {
            return;
        }
        std::vector<std::pair<pointer, pointer>> pages;
        organize_memory_page_groups(this->process, this->ignore_swapped_page, this->ignore_swapped_page, this->range, pages);
        std::for_each(pages.begin(), pages.end(), [&](const std::pair<pointer, pointer> &item) {

        });
    }
}



