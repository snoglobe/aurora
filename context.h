//
// Created by snwy on 1/22/23.
//

#ifndef AURORA_CONTEXT_H
#define AURORA_CONTEXT_H

#include "lexer.h"
#include "aurora_obj.h"
#include <stack>
#include "instruction.h"
#include "aurora_exception.h"

class AuroraContext {
    Lexer scanner;
    std::unordered_map<std::string, AuroraObj>& globals;
    std::vector<std::unordered_map<std::string, AuroraObj>> locals;

    [[nodiscard]] TokenType peek() const { return current.type; }

    bool ignoreNewlines = false;

    [[nodiscard]] bool peek(TokenType type) const { return current.type == type; }

    Token eat(TokenType type);

    AuroraObj lookupVariable(const std::string &name) {
        // decend downards through locals
        // if not found, return global
        if(!locals.empty()) {
            for (int i = locals.size() - 1; i >= 0; i--) {
                if(locals[i].find(name) != locals[i].end()) {
                    return locals[i][name];
                }
            }
        }
        if (globals.find(name) != globals.end()) {
            return globals[name];
        } else {
            throw AuroraException("Undefined variable '" + name + "'.");
        }
    }

    void setVariable(const std::string &name, const AuroraObj& value) {
        if (locals.empty()) {
            globals[name] = value;
            return;
        }
        for (int i = locals.size() - 1; i >= 0; i--) {
            if(locals[i].find(name) != locals[i].end()) {
                locals[i][name] = value;
                return;
            }
        }
    }

public:
    Token current;

    AuroraObj execute(AuroraCodeUnit unit);

    explicit AuroraContext(std::string source, std::unordered_map<std::string, AuroraObj>& globals) : scanner(std::move(source)), current(scanner.nextToken()), globals(globals) {}

    void run();

    int exprList();

    std::vector<std::string> idList();

    void primary();

    void call();

    void unary();

    void factor();

    void term();

    void comparison();

    void equality();

    void andExpr();

    void expression();

    void if_statement();

    void while_statement();

    void for_statement();

    void function_statement();

    static bool isAssignOp(TokenType type) {
        return type == TokenType::ASSIGN
               || type == TokenType::PLUS_ASSIGN
               || type == TokenType::MINUS_ASSIGN
               || type == TokenType::STAR_ASSIGN
               || type == TokenType::SLASH_ASSIGN
               || type == TokenType::MODULO_ASSIGN;
    }

    TokenType assignOp();

    void statement();
    AuroraCodeUnit currentCodeUnit = {};
};


#endif //AURORA_CONTEXT_H
