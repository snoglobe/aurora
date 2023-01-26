//
// Created by snwy on 1/23/23.
//

#ifndef AURORA_STD_LIB_H
#define AURORA_STD_LIB_H

#include <unordered_map>
#include <iostream>
#include "aurora_obj.h"

#define AURORA_FN(name, body) { name, AuroraObj(AuroraNativeFunction([](const std::vector<AuroraObj>& args) body)) }

std::unordered_map<std::string, AuroraObj> globals = {
    AURORA_FN("print", {
        for (const auto &arg : args) {
            std::cout << arg.string_representation();
        }
        std::cout << "\n";
        return AuroraObj();
    }),
    AURORA_FN("push_back", {
        if (args.size() != 2) throw AuroraException("Expected 2 arguments, got " + std::to_string(args.size()) + ".");
        guardType(args[0].value.index(), 3);
        guardType(args[1].value.index(), 0);
        auto vec = std::get<std::vector<AuroraObj>>(args[0].value);
        vec.push_back(args[1]);
        return AuroraObj(std::move(vec));
    }),
    AURORA_FN("pop_back", {
        if (args.size() != 1) throw AuroraException("Expected 1 argument, got " + std::to_string(args.size()) + ".");
        guardType(args[0].value.index(), 3);
        auto list = std::get<std::vector<AuroraObj>>(args[0].value);
        if (list.empty()) throw AuroraException("Cannot pop from empty list.");
        list.pop_back();
        return AuroraObj(std::move(list));
    }),
    AURORA_FN("size", {
        if (args.size() != 1) throw AuroraException("Expected 1 argument, got " + std::to_string(args.size()) + ".");
        guardType(args[0].value.index(), 3);
        return AuroraObj((double)std::get<std::vector<AuroraObj>>(args[0].value).size());
    }),
    AURORA_FN("range", {
        if (args.empty() || args.size() > 3) throw AuroraException("Expected 1 to 3 arguments, got " + std::to_string(args.size()) + ".");
        for (const auto &arg : args) {
            guardType(arg.value.index(), 0);
        }
        std::vector<AuroraObj> list;
        if (args.size() == 1) {
            for (int i = 0; i < args[0].asDouble(); i++) {
                list.emplace_back((double)i);
            }
        } else if (args.size() == 2) {
            for (int i = args[0].asDouble(); i < args[1].asDouble(); i++) {
                list.emplace_back((double)i);
            }
        } else {
            for (int i = args[0].asDouble(); i < args[1].asDouble(); i += args[2].asDouble()) {
                list.emplace_back((double)i);
            }
        }
        return AuroraObj(std::move(list));
    }),
    // string parsing functions
    AURORA_FN("split", {
        if (args.size() != 2) throw AuroraException("Expected 2 arguments, got " + std::to_string(args.size()) + ".");
        guardType(args[0].value.index(), 1);
        guardType(args[1].value.index(), 1);
        std::vector<AuroraObj> list;
        std::string str = std::get<std::string>(args[0].value);
        std::string delimiter = std::get<std::string>(args[1].value);
        size_t pos;
        std::string token;
        while ((pos = str.find(delimiter)) != std::string::npos) {
            token = str.substr(0, pos);
            list.emplace_back(std::move(token));
            str.erase(0, pos + delimiter.length());
        }
        list.emplace_back(std::move(str));
        return AuroraObj(std::move(list));
    }),
    AURORA_FN("join", {
        if (args.size() != 2) throw AuroraException("Expected 2 arguments, got " + std::to_string(args.size()) + ".");
        guardType(args[0].value.index(), 3);
        guardType(args[1].value.index(), 1);
        std::string str;
        for (const auto &arg : std::get<std::vector<AuroraObj>>(args[0].value)) {
            guardType(arg.value.index(), 1);
            str += std::get<std::string>(arg.value);
        }
        return AuroraObj(std::move(str));
    }),
    AURORA_FN("replace", {
        if (args.size() != 3) throw AuroraException("Expected 3 arguments, got " + std::to_string(args.size()) + ".");
        guardType(args[0].value.index(), 1);
        guardType(args[1].value.index(), 1);
        guardType(args[2].value.index(), 1);
        std::string str = std::get<std::string>(args[0].value);
        std::string from = std::get<std::string>(args[1].value);
        std::string to = std::get<std::string>(args[2].value);
        size_t start_pos = str.find(from);
        if (start_pos == std::string::npos) return AuroraObj(std::move(str));
        str.replace(start_pos, from.length(), to);
        return AuroraObj(std::move(str));
    }),
    AURORA_FN("substr", {
        if (args.size() != 3) throw AuroraException("Expected 3 arguments, got " + std::to_string(args.size()) + ".");
        guardType(args[0].value.index(), 1);
        guardType(args[1].value.index(), 0);
        guardType(args[2].value.index(), 0);
        std::string str = std::get<std::string>(args[0].value);
        int start = args[1].asDouble();
        int end = args[2].asDouble();
        if (start < 0) start = str.length() + start;
        if (end < 0) end = str.length() + end;
        if (start < 0 || end < 0 || start > end || end > str.length()) throw AuroraException("Invalid substring range.");
        return AuroraObj(str.substr(start, end - start));
    }),
    AURORA_FN("find", {
        if (args.size() != 2) throw AuroraException("Expected 2 arguments, got " + std::to_string(args.size()) + ".");
        guardType(args[0].value.index(), 1);
        guardType(args[1].value.index(), 1);
        std::string str = std::get<std::string>(args[0].value);
        std::string substr = std::get<std::string>(args[1].value);
        return AuroraObj((double)str.find(substr));
    }),
    AURORA_FN("find_last", {
        if (args.size() != 2) throw AuroraException("Expected 2 arguments, got " + std::to_string(args.size()) + ".");
        guardType(args[0].value.index(), 1);
        guardType(args[1].value.index(), 1);
        std::string str = std::get<std::string>(args[0].value);
        std::string substr = std::get<std::string>(args[1].value);
        return AuroraObj((double)str.rfind(substr));
    }),
    AURORA_FN("contains?", {
        if (args.size() != 2) throw AuroraException("Expected 2 arguments, got " + std::to_string(args.size()) + ".");
        guardType(args[0].value.index(), 1);
        guardType(args[1].value.index(), 1);
        std::string str = std::get<std::string>(args[0].value);
        std::string substr = std::get<std::string>(args[1].value);
        return AuroraObj(str.find(substr) != std::string::npos);
    }),
    AURORA_FN("empty?", {
        if (args.size() != 1) throw AuroraException("Expected 1 argument, got " + std::to_string(args.size()) + ".");
        guardType(args[0].value.index(), 1);
        std::string str = std::get<std::string>(args[0].value);
        return AuroraObj(str.empty());
    }),
    AURORA_FN("to_string", {
        if (args.size() != 1) throw AuroraException("Expected 1 argument, got " + std::to_string(args.size()) + ".");
        return AuroraObj(args[0].string_representation());
    }),
    // input
    AURORA_FN("input", {
        if (!args.empty()) throw AuroraException("Expected 0 arguments, got " + std::to_string(args.size()) + ".");
        std::string str;
        std::getline(std::cin, str);
        return AuroraObj(std::move(str));
    }),
    {"NaN", AuroraObj(std::numeric_limits<double>::quiet_NaN())},
    AURORA_FN("input_int", {
        if (!args.empty()) throw AuroraException("Expected 0 arguments, got " + std::to_string(args.size()) + ".");
        std::string str;
        std::getline(std::cin, str);
        try {
            return AuroraObj(std::stod(str));
        } catch (std::invalid_argument &) {
            return AuroraObj(std::numeric_limits<double>::quiet_NaN());
        }
    }),
    AURORA_FN("input_double", {
        if (!args.empty()) throw AuroraException("Expected 0 arguments, got " + std::to_string(args.size()) + ".");
        std::string str;
        std::getline(std::cin, str);
        try {
            return AuroraObj(std::stod(str));
        } catch (std::invalid_argument &) {
            return AuroraObj(std::numeric_limits<double>::quiet_NaN());
        }
    }),
    AURORA_FN("input_bool", {
        if (!args.empty()) throw AuroraException("Expected 0 arguments, got " + std::to_string(args.size()) + ".");
        std::string str;
        std::getline(std::cin, str);
        if (str == "true") return AuroraObj(true);
        if (str == "false") return AuroraObj(false);
        return AuroraObj();
    }),
    AURORA_FN("read?", {
        // test if stdin matches args[0]
        // without reading a full line
        // read up to the next whitespace
        // return true if it does, false if it doesn't, and nil if there is no more input
        if (args.size() != 1) throw AuroraException("Expected 1 argument, got " + std::to_string(args.size()) + ".");
        guardType(args[0].value.index(), 1);
        std::string str = std::get<std::string>(args[0].value);
        std::string input;
        std::cin >> input;
        if (std::cin.eof()) return AuroraObj();
        return AuroraObj(input == str);
    }),
    AURORA_FN("read_delim?", {
        // test if stdin matches args[0]
        // without reading a full line
        // read up to the next delimiter
        // return true if it does, false if it doesn't, and nil if there is no more input
        if (args.size() != 1) throw AuroraException("Expected 1 argument, got " + std::to_string(args.size()) + ".");
        guardType(args[0].value.index(), 1);
        std::string str = std::get<std::string>(args[0].value);
        std::string input;
        std::getline(std::cin, input, str[0]);
        if (std::cin.eof()) return AuroraObj();
        return AuroraObj(input == str);
    }),

};

#endif //AURORA_STD_LIB_H
