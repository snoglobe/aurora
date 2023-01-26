//
// Created by snwy on 1/22/23.
//

#ifndef AURORA_AURORA_EXCEPTION_H
#define AURORA_AURORA_EXCEPTION_H

#include <string>

struct AuroraObj;

class AuroraException : public std::exception {
    std::string message;
public:
    explicit AuroraException(std::string message) : message(std::move(message)) {}

    [[nodiscard]] const char *what() const noexcept override {
        return message.c_str();
    }
};

class InternalAuroraException : public std::exception {
public:
    enum class Type {
        BREAK, CONTINUE
    };
    Type type;

    explicit InternalAuroraException(Type type) : type(type){}
};
#endif //AURORA_AURORA_EXCEPTION_H
