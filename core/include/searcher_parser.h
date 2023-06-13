#ifndef HAK_SEARCHER_PARSER_H
#define HAK_SEARCHER_PARSER_H

#include "types.h"

#include <vector>
#include <string>

namespace hak {
    enum value_type {
        type_i8 = 0,
        type_i16 = 1,
        type_i32 = 2,
        type_i64 = 3,
        type_u8 = 4,
        type_u16 = 5,
        type_u32 = 6,
        type_u64 = 7,
        type_float = 8,
        type_double = 9,
        type_range = 10,
        type_unknown = 11
    };
    using basic_value = std::variant<i8, i16, i32, i64, u8, u16, u32, u64, double, float>;

    class range {
    public:
        basic_value start;
        basic_value end;
        value_type type = type_unknown;
    };

    using value = std::variant<i8, i16, i32, i64, u8, u16, u32, u64, double, float, range>;

    auto parse_search_number_expr(const std::string& expr, value_type default_type) -> std::vector<value>;
}


#endif //HAK_SEARCHER_PARSER_H
