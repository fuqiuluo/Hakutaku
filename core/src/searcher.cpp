#include "searcher.h"

#include <vector>
#include <unordered_set>
#include <functional>
#include <unistd.h>

namespace hak {
    memory_searcher::memory_searcher(std::shared_ptr<hak::process> process) {
        this->process = process;
    }

    void memory_searcher::set_memory_range(i32 _range) {
        this->range = _range;
    }

    void memory_searcher::set_ignore_swapped_page(bool ignore) {
        this->config.ignore_swapped_page = ignore;
    }

    void memory_searcher::set_ignore_missing_page(bool ignore) {
        this->config.ignore_missing_page = ignore;
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

    auto hak::memory_searcher::organize_memory_page_groups(std::vector<std::pair<pointer, pointer>>& dest) {
        std::vector<std::pair<pointer, pointer>> pages;
        auto maps = this->process->get_maps(range);
        do {
            get_legal_pages(process, maps, this->config.ignore_swapped_page, this->config.ignore_missing_page, pages);
        } while ((maps = maps->next()));
        pointer start = 0;
        pointer end = 0;
        std::for_each(pages.begin(), pages.end(), [&](const std::pair<pointer, pointer> &item) {
            if (this->config.end == 0 || (item.first >= this->config.start && item.second <= this->config.end)) {
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
            }
        });
        //std::cout << "organized legal page size = " << dest.size() << "\n";
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

    auto get_value_size_by_type(value_type type) -> size_t {
        switch (type) {
            case type_i8: {
                return sizeof(i8);
            }
            case type_i16: {
                return sizeof(i16);
            }
            case type_i32: {
                return sizeof(i32);
            }
            case type_i64: {
                return sizeof(i64);
            }
            case type_u8: {
                return sizeof(u8);
            }
            case type_u16: {
                return sizeof(u16);
            }
            case type_u32: {
                return sizeof(u32);
            }
            case type_u64: {
                return sizeof(u64);
            }
            case type_float: {
                return sizeof(float);
            }
            case type_double: {
                return sizeof(double);
            }
            case type_range:
            case type_unknown:
                break;
        }
        return 0;
    }

    auto get_value_size(value value) -> size_t {
        if (value.index() == type_range) {
            auto range = std::get<hak::range>(value);
            return get_value_size_by_type(range.type);
        }
        return get_value_size_by_type(static_cast<value_type>(value.index()));
    }

    auto scan_value(std::shared_ptr<process>& process, const std::pair<pointer, pointer>& pair, size_t size, // NOLINT(*-no-recursion)
                    std::vector<value>::iterator value, match_sign sign, std::unordered_set<pointer>& result, i32 depth, std::function<void(pointer addr)> callback) -> bool {
        //std::vector<value>::iterator value, match_sign sign, std::vector<pointer>& result, i32 depth, std::function<void(pointer addr)> callback) -> bool {
        auto start = pair.first;
        auto end = pair.second;
        auto value_size = get_value_size(*value);
        //std::cout << "value size = " << value_size << ", depth = " << depth << ", index = " << value->index() << "\n";
        pointer addr = start;
        do {
            if (match_value(process, *value, addr, sign)) {
                if (depth >= size - 2) {
                    callback(addr);
                    return true;
                }
                auto next = std::next(value);
                scan_value(process, std::make_pair(addr, end), size, next, sign, result, depth + 1, [&](pointer _addr) {
                    result.insert(_addr);
                    //result.emplace_back(_addr);
                    callback(addr);
                });
            }
            addr += value_size; // NOLINT(*-narrowing-conversions)
        } while (addr <= end && (end - addr) >= value_size);
        return false;
    }

    auto memory_searcher::searchNumber(const std::string &expr, value_type default_type, match_sign sign) -> size_t {
        auto values = hak::parse_search_number_expr(expr, default_type);
        if (values.empty()) {
            return 0;
        }
        std::vector<std::pair<pointer, pointer>> pages;
        this->organize_memory_page_groups(pages);
        std::for_each(pages.begin(), pages.end(), [&](const std::pair<pointer, pointer> &item) {
            auto start = item.first;
            auto end = item.second;
            auto value_size = get_value_size(values[0]);
            pointer addr = start;
            do {
                if (match_value(process, values[0], addr, sign)) {
                    std::unordered_set<pointer> result;
                    //std::vector<pointer> result;
                    scan_value(process, std::make_pair(addr, end), values.size(), std::next(values.begin()), sign, result, 0, [&](pointer callback_addr) {
                        this->results.insert(result.begin(), result.end());
                        this->results.insert(callback_addr);
                        this->results.insert(addr);
                        //std::copy(result.begin(), result.end(), std::back_inserter(this->results));
                        //this->results.emplace_back(callback_addr);
                        //this->results.emplace_back(addr);
                    });
                }
                addr += value_size; // NOLINT(*-narrowing-conversions)
            } while (addr < end && (end - addr) >= value_size);
        });
        //std::sort(this->results.begin(), this->results.end());
        //this->results.erase(std::unique(this->results.begin(), this->results.end()), this->results.end());
        return this->results.size();
    }

    void memory_searcher::set_search_range(pointer start, pointer end) {
        if (end < start || start < 0 || end < 0) {
            throw std::range_error("Searcher range is illegal.");
        }
        this->config.start = start;
        this->config.end = end;
    }
}



