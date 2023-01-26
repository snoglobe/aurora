//
// Created by snwy on 1/22/23.
//

#ifndef AURORA_AURORA_OBJ_H
#define AURORA_AURORA_OBJ_H

#include <variant>
#include <string>
#include <vector>
#include <functional>
#include <cmath>
#include "instruction.h"
#include "aurora_exception.h"

struct AuroraObj;

struct AuroraCodeUnit {
    std::vector<Instruction> instructions;
    std::vector<AuroraObj> constants;

    void emit(InstructionType type, int operand = 0) {
        instructions.push_back({type, operand});
    }

    int getConstantIndex(const AuroraObj &obj);
};

struct AuroraFunction {
    std::vector<std::string> parameters;
    AuroraCodeUnit code;
};

inline std::string variantIndexToString(size_t index) {
    switch (index) {
        case 0: return "number";
        case 1: return "string";
        case 2: return "bool";
        case 3: return "list";
        case 4: return "function";
        case 5: return "code unit";
        case 6: return "null";
        case 7: return "native function";
    }
    return "unknown";
}

inline void guardType(size_t index, size_t expected) {
    if (index != expected) throw AuroraException("Expected " + variantIndexToString(expected) + ", got " + variantIndexToString(index) + ".");
}

typedef std::function<AuroraObj(std::vector<AuroraObj>)> AuroraNativeFunction;

struct AuroraObj {
    std::variant<double, std::string, bool, std::vector<AuroraObj>, AuroraFunction, AuroraCodeUnit, std::monostate, AuroraNativeFunction> value{};

    explicit AuroraObj() : value(std::monostate{}) {}

    explicit AuroraObj(double value) : value(value) {}

    explicit AuroraObj(std::string value) : value(std::move(value)) {}

    explicit AuroraObj(bool value) : value(value) {}

    explicit AuroraObj(std::vector<AuroraObj> value) : value(std::move(value)) {}

    explicit AuroraObj(AuroraFunction value) : value(std::move(value)) {}

    explicit AuroraObj(AuroraCodeUnit value) : value(std::move(value)) {}

    explicit AuroraObj(AuroraNativeFunction value) : value(std::move(value)) {}

    [[nodiscard]] double asDouble() const { guardType(value.index(), 0); return std::get<double>(value); }

    [[nodiscard]] std::string asString() const { guardType(value.index(), 1); return std::get<std::string>(value); }

    [[nodiscard]] bool asBool() const { guardType(value.index(), 2); return std::get<bool>(value); }

    [[nodiscard]] std::vector<AuroraObj> asVector() const { guardType(value.index(), 3); return std::get<std::vector<AuroraObj>>(value); }

    [[nodiscard]] AuroraFunction asFunction() const { guardType(value.index(), 4); return std::get<AuroraFunction>(value); }

    [[nodiscard]] AuroraCodeUnit asCodeUnit() const { guardType(value.index(), 5); return std::get<AuroraCodeUnit>(value); }

    [[nodiscard]] AuroraNativeFunction asNativeFunction() const { guardType(value.index(), 7); return std::get<AuroraNativeFunction>(value); }

    bool operator==(const AuroraObj &other) const {
        if (value.index() != other.value.index()) return false;
        switch (value.index()) {
            case 0:
                return asDouble() == other.asDouble();
            case 1:
                return asString() == other.asString();
            case 2:
                return asBool() == other.asBool();
            case 3:
                return asVector() == other.asVector();
            case 4:
            case 5:
                return false;
            default:
                throw AuroraException("Invalid AuroraObj type.");
        }
    }

    bool operator!=(const AuroraObj &other) const { return !(*this == other); }
    AuroraObj& operator=(const AuroraObj& other) = default;

    [[nodiscard]] std::string string_representation() const {
        switch (value.index()) {
            case 0: {
                std::string result = std::to_string(asDouble());
                if (result.find('.') != std::string::npos) {
                    while (result.back() == '0') result.pop_back();
                    if (result.back() == '.') result.pop_back();
                }
                return result;
            }
            case 1:
                return std::get<std::string>(value);
            case 2:
                return asBool() ? "true" : "false";
            case 3: {
                if (asVector().empty()) return "[]";
                std::string result = "{";
                for (const auto &obj: asVector()) {
                    result += obj.string_representation() + ", ";
                }
                result.pop_back();
                result.pop_back();
                result += "}";
                return result;
            }
            case 4:
                return "function";
            case 5:
                return "code unit";
            case 6:
                return "null";
            case 7:
                return "native function";
            default:
                throw AuroraException("Invalid AuroraObj type.");
        }
    }
};

inline int AuroraCodeUnit::getConstantIndex(const AuroraObj &obj)  {
    if(obj.value.index() < 4) {
        for (int i = 0; i < constants.size(); i++) {
            if(constants[i].value.index() == obj.value.index()) {
                switch (constants[i].value.index()) {
                    case 0:
                        if (std::get<double>(constants[i].value) == std::get<double>(obj.value)) return i;
                        break;
                    case 1:
                        if (std::get<std::string>(constants[i].value) == std::get<std::string>(obj.value)) return i;
                        break;
                    case 2:
                        if (std::get<bool>(constants[i].value) == std::get<bool>(obj.value)) return i;
                        break;
                    case 3:
                        if (std::get<std::vector<AuroraObj>>(constants[i].value) ==
                            std::get<std::vector<AuroraObj>>(obj.value))
                            return i;
                        break;
                    default:
                        break;
                }
            }
        }
    }
    constants.push_back(obj);
    return constants.size() - 1;
}

class AuroraReturnException : public std::exception {
public:
    AuroraObj value;

    explicit AuroraReturnException(AuroraObj value) : value(value){}
};



#endif //AURORA_AURORA_OBJ_H
