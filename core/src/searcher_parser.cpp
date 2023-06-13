#include "searcher_parser.h"

#include <sstream>
#include <istream>
#include <iostream>
#include <string>
#include <cctype>

namespace hak {
    auto determine_type(std::string& token) -> value_type {
        auto type_str = token.substr(token.size() - 1);
        if (type_str == "D" || type_str == "d") {
            return type_i32;
        }
        if (type_str == "F" || type_str == "f") {
            return type_float;
        }
        if (type_str == "B" || type_str == "b") {
            return type_i8;
        }
        if (type_str == "W" || type_str == "w") {
            return type_i16;
        }
        if (type_str == "Q" || type_str == "q") {
            return type_i64;
        }
        if (type_str == "E" || type_str == "e") {
            return type_double;
        }
        return type_unknown;
    }

    void parse_and_push(std::string& value_str, value_type type, bool is_unsigned, std::vector<value>& values) {
        if (type == type_i32) {
            auto num = std::stoul(value_str);
            if (is_unsigned) {
                //std::cout << "push = " << num << "\n";
                values.emplace_back(((u32) num));
            } else {
                values.emplace_back(((i32) num));
            }
        } else if (type == type_float) {
            values.emplace_back(std::stof(value_str));
        } else if (type == type_i8) {
            auto num = std::stoul(value_str);
            if (is_unsigned) {
                values.emplace_back(((u8) num));
            } else {
                //std::cout << "push i8 = " << num << "\n";
                values.emplace_back(((i8) num));
            }
        } else if (type == type_i16) {
            auto num = std::stoul(value_str);
            if (is_unsigned) {
                values.emplace_back(((u16) num));
            } else {
                values.emplace_back(((i16) num));
            }
        } else if (type == type_i64) {
            auto num = std::stoll(value_str);
            if (is_unsigned) {
                values.emplace_back(((u64) num));
            } else {
                values.emplace_back(((i64) num));
            }
        } else if (type == type_double) {
            values.emplace_back(std::stod(value_str));
        } else {
            throw std::runtime_error("Out of number range.");
        }
    }

    void parse_and_pushv2(std::string& value_str, value_type type, bool is_unsigned, range& range, bool is_start) {
        basic_value *target = is_start ? &(range.start) : &(range.end);
        if (type == type_i32) {
            auto num = std::stoul(value_str);
            if (is_unsigned) {
                *target = (u32) num;
            } else {
                *target = (i32) num;
            }
        } else if (type == type_float) {
            *target = std::stof(value_str);
        } else if (type == type_i8) {
            auto num = std::stoul(value_str);
            if (is_unsigned) {
                *target = (u8) num;
            } else {
                *target = (i8) num;
            }
        } else if (type == type_i16) {
            auto num = std::stoul(value_str);
            if (is_unsigned) {
                *target = (u16) num;
            } else {
                *target = (i16) num;
            }
        } else if (type == type_i64) {
            auto num = std::stoll(value_str);
            if (is_unsigned) {
                *target = (u64) num;
            } else {
                *target = (i64) num;
            }
        } else if (type == type_double) {
            *target = std::stod(value_str);
        } else {
            throw std::runtime_error("Out of number range.");
        }
    }

    auto parse_search_number_expr(const std::string &expr, value_type default_type) -> std::vector<value> {
        std::istringstream iss(expr);
        std::string token;
        std::vector<value> values;

        while (getline(iss, token, ';')) {
            if (token.find('~') != std::string::npos) {
                auto index = token.find('~');
                auto start_token = token.substr(0, index);
                auto end_token = token.substr(index + 1);
                value_type start_type = determine_type(start_token);
                value_type end_type = determine_type(end_token);
                value_type type = start_type == type_unknown ? end_type : start_type;
                range range;
                auto start = start_type == type_unknown ? start_token : start_token.substr(0, token.size() - 1);
                auto end = end_type == type_unknown ? end_token : end_token.substr(0, token.size() - 1);
                bool is_unsigned = std::isdigit(start[0]) != 0;
                parse_and_pushv2(start, type == type_unknown ? default_type : type, is_unsigned, range, true);
                parse_and_pushv2(end, type == type_unknown ? default_type : type, is_unsigned, range, false);
                range.type = type;
                values.emplace_back(range);
            } else {
                auto value_str = token.substr(0, token.size() - 1);
                auto type = determine_type(token);
                // std::cout << "type = " << type << "\n";
                bool is_unsigned = std::isdigit(value_str[0]) != 0;
                parse_and_push(value_str, type == type_unknown ? default_type : type, is_unsigned, values);
            }
        }
        return std::move(values);
    }
}