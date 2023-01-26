//
// Created by snwy on 1/22/23.
//

#include "context.h"
#include <iostream>

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::LEFT_PAREN:
            return "LEFT_PAREN";
        case TokenType::RIGHT_PAREN:
            return "RIGHT_PAREN";
        case TokenType::LEFT_BRACE:
            return "LEFT_BRACE";
        case TokenType::RIGHT_BRACE:
            return "RIGHT_BRACE";
        case TokenType::COMMA:
            return "COMMA";
        case TokenType::MINUS:
            return "MINUS";
        case TokenType::PLUS:
            return "PLUS";
        case TokenType::SLASH:
            return "SLASH";
        case TokenType::STAR:
            return "STAR";
        case TokenType::MODULO:
            return "MODULO";
        case TokenType::LESS:
            return "LESS";
        case TokenType::LESS_EQUAL:
            return "LESS_EQUAL";
        case TokenType::GREATER:
            return "GREATER";
        case TokenType::GREATER_EQUAL:
            return "GREATER_EQUAL";
        case TokenType::EQUAL:
            return "EQUAL";
        case TokenType::NOT_EQUAL:
            return "NOT_EQUAL";
        case TokenType::AND:
            return "AND";
        case TokenType::OR:
            return "OR";
        case TokenType::NOT:
            return "NOT";
        case TokenType::COLON:
            return "COLON";
        case TokenType::IDENTIFIER:
            return "IDENTIFIER";
        case TokenType::STRING:
            return "STRING";
        case TokenType::NUMBER:
            return "NUMBER";
        case TokenType::TRUE:
            return "TRUE";
        case TokenType::FALSE:
            return "FALSE";
        case TokenType::NIL:
            return "NIL";
        case TokenType::IF:
            return "IF";
        case TokenType::ELSE:
            return "ELSE";
        case TokenType::WHILE:
            return "WHILE";
        case TokenType::FOR:
            return "FOR";
        case TokenType::FN:
            return "FN";
        case TokenType::RETURN:
            return "RETURN";
        case TokenType::BREAK:
            return "BREAK";
        case TokenType::CONTINUE:
            return "CONTINUE";
        case TokenType::NEWLINE:
            return "NEWLINE";
        case TokenType::END:
            return "END";
        case TokenType::ASSIGN:
            return "ASSIGN";
        case TokenType::PLUS_ASSIGN:
            return "PLUS_ASSIGN";
        case TokenType::MINUS_ASSIGN:
            return "MINUS_ASSIGN";
        case TokenType::STAR_ASSIGN:
            return "STAR_ASSIGN";
        case TokenType::SLASH_ASSIGN:
            return "SLASH_ASSIGN";
        case TokenType::MODULO_ASSIGN:
            return "MODULO_ASSIGN";
        case TokenType::ARROW:
            return "ARROW";
        case TokenType::EOF_:
            return "EOF";
    }
    return "UNKNOWN";
}

double dmod(double x, double y) {
    return x - (int) (x / y) * y;
}

void AuroraContext::run() {
    while (current.type != TokenType::EOF_) {
        statement();
    }
    execute(currentCodeUnit);
}

int AuroraContext::exprList() {
    int count = 1;
    expression();
    while (peek(TokenType::COMMA)) {
        eat(TokenType::COMMA);
        expression();
        count++;
    }
    return count;
}

Token AuroraContext::eat(TokenType type) {
    if (ignoreNewlines) {
        while (peek(TokenType::NEWLINE)) current = scanner.nextToken();
    }
    if (current.type == type) {
        Token token = current;
        current = scanner.nextToken();
        return token;
    }
    throw AuroraException(
            "Unexpected token " + tokenTypeToString(peek()) + " at line " + std::to_string(current.line) +
            ", expected " +
            tokenTypeToString(type) + ".");
}

std::vector<std::string> AuroraContext::idList() {
    std::vector<std::string> ids;
    ids.push_back(eat(TokenType::IDENTIFIER).lexeme);
    while (peek(TokenType::COMMA)) {
        eat(TokenType::COMMA);
        ids.push_back(eat(TokenType::IDENTIFIER).lexeme);
    }
    return ids;
}

void AuroraContext::primary() {
    switch (peek()) {
        case TokenType::NUMBER: {
            auto value = std::any_cast<double>(eat(TokenType::NUMBER).literal);
            if (value == (int) value) {
                currentCodeUnit.emit(InstructionType::PUSHI, (int) value);
            } else {
                currentCodeUnit.emit(InstructionType::PUSH, currentCodeUnit.getConstantIndex(AuroraObj(value)));
            }
            break;
        }
        case TokenType::STRING: {
            currentCodeUnit.emit(InstructionType::PUSH, currentCodeUnit.getConstantIndex(
                    AuroraObj(eat(TokenType::STRING).lexeme)));
            break;
        }
        case TokenType::TRUE: {
            eat(TokenType::TRUE);
            currentCodeUnit.emit(InstructionType::TRUE);
            break;
        }
        case TokenType::FALSE: {
            eat(TokenType::FALSE);
            currentCodeUnit.emit(InstructionType::FALSE);
            break;
        }
        case TokenType::LEFT_PAREN: {
            eat(TokenType::LEFT_PAREN);
            bool prev = ignoreNewlines;
            ignoreNewlines = true;
            expression();
            ignoreNewlines = prev;
            eat(TokenType::RIGHT_PAREN);
            break;
        }
        case TokenType::IDENTIFIER: {
            currentCodeUnit.emit(InstructionType::LOAD, currentCodeUnit.getConstantIndex(
                    AuroraObj(eat(TokenType::IDENTIFIER).lexeme)));
            break;
        }
        case TokenType::LEFT_BRACE: {
            eat(TokenType::LEFT_BRACE);
            bool prev = ignoreNewlines;
            ignoreNewlines = true;
            int count = 0;
            if (!peek(TokenType::RIGHT_BRACE)) {
                expression();
                count++;
                while (peek(TokenType::COMMA) || peek(TokenType::NEWLINE)) {
                    eat(TokenType::COMMA);
                    expression();
                    count++;
                }
            }
            ignoreNewlines = prev;
            eat(TokenType::RIGHT_BRACE);
            currentCodeUnit.emit(InstructionType::LIST, count);
            break;
        }
        default:
            throw AuroraException("Unexpected token " + tokenTypeToString(current.type) + " at line " +
                                  std::to_string(current.line));
    }
}

void AuroraContext::call() {
    primary();
    if (peek(TokenType::LEFT_PAREN)) {
        eat(TokenType::LEFT_PAREN);
        int count;
        if (!peek(TokenType::RIGHT_PAREN)) {
            count = exprList();
        } else {
            count = 0;
        }
        eat(TokenType::RIGHT_PAREN);
        currentCodeUnit.emit(InstructionType::CALL, count);
    } else if (peek(TokenType::COLON)) {
        eat(TokenType::COLON);
        primary();
        currentCodeUnit.emit(InstructionType::IDX);
    }
}

void AuroraContext::unary() {
    if (peek(TokenType::NOT) || peek(TokenType::MINUS)) {
        auto op = eat(peek(TokenType::NOT) ? TokenType::NOT : TokenType::MINUS);
        unary();
        if (op.type == TokenType::NOT) currentCodeUnit.emit(InstructionType::NOT);
        else currentCodeUnit.emit(InstructionType::NEG);
    } else {
        call();
    }
}

void AuroraContext::factor() {
    unary();
    while (peek(TokenType::STAR) || peek(TokenType::SLASH) || peek(TokenType::MODULO)) {
        auto op = eat(peek(TokenType::STAR) ? TokenType::STAR : (peek(TokenType::SLASH) ? TokenType::SLASH
                                                                                        : TokenType::MODULO));
        unary();
        if (op.type == TokenType::STAR) currentCodeUnit.emit(InstructionType::MUL);
        else if (op.type == TokenType::SLASH) currentCodeUnit.emit(InstructionType::DIV);
        else currentCodeUnit.emit(InstructionType::MOD);
    }
}

void AuroraContext::term() {
    factor();
    while (peek(TokenType::PLUS) || peek(TokenType::MINUS)) {
        auto op = eat(peek(TokenType::PLUS) ? TokenType::PLUS : TokenType::MINUS);
        factor();
        if (op.type == TokenType::PLUS) currentCodeUnit.emit(InstructionType::ADD);
        else currentCodeUnit.emit(InstructionType::SUB);
    }
}

void AuroraContext::comparison() {
    term();
    while (peek(TokenType::GREATER) || peek(TokenType::GREATER_EQUAL) || peek(TokenType::LESS) ||
           peek(TokenType::LESS_EQUAL)) {
        auto op = eat(peek(TokenType::GREATER) ? TokenType::GREATER : peek(TokenType::GREATER_EQUAL)
                                                                      ? TokenType::GREATER_EQUAL : peek(
                        TokenType::LESS) ? TokenType::LESS : TokenType::LESS_EQUAL);
        term();
        if (op.type == TokenType::GREATER) currentCodeUnit.emit(InstructionType::GT);
        else if (op.type == TokenType::GREATER_EQUAL) currentCodeUnit.emit(InstructionType::GTE);
        else if (op.type == TokenType::LESS) currentCodeUnit.emit(InstructionType::LT);
        else currentCodeUnit.emit(InstructionType::LTE);
    }
}

void AuroraContext::equality() {
    comparison();
    while (peek(TokenType::NOT_EQUAL) || peek(TokenType::EQUAL)) {
        auto op = eat(peek(TokenType::NOT_EQUAL) ? TokenType::NOT_EQUAL : TokenType::EQUAL);
        comparison();
        if (op.type == TokenType::NOT_EQUAL) currentCodeUnit.emit(InstructionType::NEQ);
        else currentCodeUnit.emit(InstructionType::EQ);
    }
}

void AuroraContext::andExpr() {
    equality();
    while (peek(TokenType::AND)) {
        eat(TokenType::AND);
        equality();
        currentCodeUnit.emit(InstructionType::AND);
    }
}

void AuroraContext::expression() {
    andExpr();
    while (peek(TokenType::OR)) {
        eat(TokenType::OR);
        andExpr();
        currentCodeUnit.emit(InstructionType::OR);
    }
}

void AuroraContext::if_statement() {
    eat(TokenType::IF);
    expression();
    if (peek(TokenType::NEWLINE)) {
        eat(TokenType::NEWLINE);
        auto prev = currentCodeUnit;
        currentCodeUnit = AuroraCodeUnit();
        while (!peek(TokenType::ELSE) && !peek(TokenType::END)) {
            statement();
        }
        auto ifBlock = currentCodeUnit;
        auto elseBlock = AuroraCodeUnit();
        if (peek(TokenType::ELSE)) {
            eat(TokenType::ELSE);
            eat(TokenType::NEWLINE);
            currentCodeUnit = AuroraCodeUnit();
            while (!peek(TokenType::END)) {
                statement();
            }
            elseBlock = currentCodeUnit;
        }
        currentCodeUnit = prev;
        eat(TokenType::END);
        eat(TokenType::NEWLINE);
        currentCodeUnit.emit(InstructionType::PUSH, currentCodeUnit.getConstantIndex(AuroraObj(elseBlock)));
        currentCodeUnit.emit(InstructionType::PUSH, currentCodeUnit.getConstantIndex(AuroraObj(ifBlock)));
        currentCodeUnit.emit(InstructionType::IF);
    } else {
        auto prev = currentCodeUnit;
        currentCodeUnit = AuroraCodeUnit();
        statement();
        auto ifBlock = currentCodeUnit;
        auto elseBlock = AuroraCodeUnit();
        if (peek(TokenType::ELSE)) {
            eat(TokenType::ELSE);
            currentCodeUnit = AuroraCodeUnit();
            statement();
            elseBlock = currentCodeUnit;
        }
        currentCodeUnit = prev;
        currentCodeUnit.emit(InstructionType::PUSH, currentCodeUnit.getConstantIndex(AuroraObj(elseBlock)));
        currentCodeUnit.emit(InstructionType::PUSH, currentCodeUnit.getConstantIndex(AuroraObj(ifBlock)));
        currentCodeUnit.emit(InstructionType::IF);
    }
}

void AuroraContext::while_statement() {
    eat(TokenType::WHILE);
    AuroraCodeUnit cond;
    auto prev = currentCodeUnit;
    currentCodeUnit = AuroraCodeUnit();
    expression();
    currentCodeUnit.emit(InstructionType::RES);
    cond = currentCodeUnit;
    currentCodeUnit = prev;
    if (peek(TokenType::NEWLINE)) {
        eat(TokenType::NEWLINE);
        currentCodeUnit = AuroraCodeUnit();
        while (!peek(TokenType::END)) {
            statement();
        }
        auto body = currentCodeUnit;
        currentCodeUnit = prev;
        eat(TokenType::END);
        eat(TokenType::NEWLINE);
        currentCodeUnit.emit(InstructionType::PUSH, currentCodeUnit.getConstantIndex(AuroraObj(cond)));
        currentCodeUnit.emit(InstructionType::PUSH, currentCodeUnit.getConstantIndex(AuroraObj(body)));
        currentCodeUnit.emit(InstructionType::WLOOP);
    } else {
        currentCodeUnit = AuroraCodeUnit();
        statement();
        auto body = currentCodeUnit;
        currentCodeUnit = prev;
        currentCodeUnit.emit(InstructionType::PUSH, currentCodeUnit.getConstantIndex(AuroraObj(cond)));
        currentCodeUnit.emit(InstructionType::PUSH, currentCodeUnit.getConstantIndex(AuroraObj(body)));
        currentCodeUnit.emit(InstructionType::WLOOP);
    }
}

void AuroraContext::for_statement() {
    eat(TokenType::FOR);
    auto name = eat(TokenType::IDENTIFIER).lexeme;
    eat(TokenType::COMMA);
    expression();
    auto prev = currentCodeUnit;
    currentCodeUnit = AuroraCodeUnit();
    if (peek(TokenType::NEWLINE)) {
        eat(TokenType::NEWLINE);
        while (!peek(TokenType::END)) {
            statement();
        }
        auto body = currentCodeUnit;
        currentCodeUnit = prev;
        eat(TokenType::END);
        eat(TokenType::NEWLINE);
        currentCodeUnit.emit(InstructionType::PUSH, currentCodeUnit.getConstantIndex(AuroraObj(body)));
        currentCodeUnit.emit(InstructionType::FLOOP, currentCodeUnit.getConstantIndex(AuroraObj(name)));
    } else {
        statement();
        auto body = currentCodeUnit;
        currentCodeUnit = prev;
        currentCodeUnit.emit(InstructionType::PUSH, currentCodeUnit.getConstantIndex(AuroraObj(body)));
        currentCodeUnit.emit(InstructionType::FLOOP, currentCodeUnit.getConstantIndex(AuroraObj(name)));
    }
}

void AuroraContext::function_statement() {
    eat(TokenType::FN);
    auto name = eat(TokenType::IDENTIFIER).lexeme;
    auto params = std::vector<std::string>();
    if (!peek(TokenType::NEWLINE) && !peek(TokenType::ARROW)) {
        params = idList();
    }
    if (peek(TokenType::ARROW)) {
        auto prev = currentCodeUnit;
        currentCodeUnit = AuroraCodeUnit();
        eat(TokenType::ARROW);
        expression();
        currentCodeUnit.emit(InstructionType::RET);
        auto body = currentCodeUnit;
        currentCodeUnit = prev;
        AuroraFunction fn{params, body};
        currentCodeUnit.emit(InstructionType::PUSH, currentCodeUnit.getConstantIndex(AuroraObj(fn)));
        currentCodeUnit.emit(InstructionType::STORE, currentCodeUnit.getConstantIndex(AuroraObj(name)));
    } else {
        eat(TokenType::NEWLINE);
        auto prev = currentCodeUnit;
        currentCodeUnit = AuroraCodeUnit();
        while (!peek(TokenType::END)) {
            statement();
        }
        currentCodeUnit.emit(InstructionType::PUSH, currentCodeUnit.getConstantIndex(AuroraObj()));
        currentCodeUnit.emit(InstructionType::RET);
        auto body = currentCodeUnit;
        currentCodeUnit = prev;
        eat(TokenType::END);
        eat(TokenType::NEWLINE);
        AuroraFunction fn{params, body};
        currentCodeUnit.emit(InstructionType::PUSH, currentCodeUnit.getConstantIndex(AuroraObj(fn)));
        currentCodeUnit.emit(InstructionType::STORE, currentCodeUnit.getConstantIndex(AuroraObj(name)));
    }
}

TokenType AuroraContext::assignOp() {
    if (isAssignOp(peek())) {
        return eat(peek()).type;
    }
    throw AuroraException("Expected assignment operator");
}

void AuroraContext::statement() {
    switch (peek()) {
        case TokenType::IF:
            if_statement();
            break;
        case TokenType::WHILE:
            while_statement();
            break;
        case TokenType::FOR:
            for_statement();
            break;
        case TokenType::FN:
            function_statement();
            break;
        case TokenType::RETURN:
            eat(TokenType::RETURN);
            if (peek(TokenType::NEWLINE)) {
                currentCodeUnit.emit(InstructionType::PUSH, currentCodeUnit.getConstantIndex(AuroraObj()));
                currentCodeUnit.emit(InstructionType::RET);
            } else {
                expression();
                currentCodeUnit.emit(InstructionType::RET);
                eat(TokenType::NEWLINE);
            }
            break;
        case TokenType::BREAK:
            eat(TokenType::BREAK);
            currentCodeUnit.emit(InstructionType::BREAK);
            eat(TokenType::NEWLINE);
            break;
        case TokenType::CONTINUE:
            eat(TokenType::CONTINUE);
            currentCodeUnit.emit(InstructionType::CONTINUE);
            eat(TokenType::NEWLINE);
            break;
        case TokenType::IDENTIFIER: {
            auto name = eat(TokenType::IDENTIFIER).lexeme;
            if (isAssignOp(peek())) {
                auto op = eat(peek()).type;
                expression();
                switch (op) {
                    case TokenType::ASSIGN:
                        currentCodeUnit.emit(InstructionType::STORE,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        break;
                    case TokenType::PLUS_ASSIGN:
                        currentCodeUnit.emit(InstructionType::LOAD,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        currentCodeUnit.emit(InstructionType::ADD);
                        currentCodeUnit.emit(InstructionType::STORE,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        break;
                    case TokenType::MINUS_ASSIGN:
                        currentCodeUnit.emit(InstructionType::LOAD,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        currentCodeUnit.emit(InstructionType::SUB);
                        currentCodeUnit.emit(InstructionType::STORE,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        break;
                    case TokenType::STAR_ASSIGN:
                        currentCodeUnit.emit(InstructionType::LOAD,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        currentCodeUnit.emit(InstructionType::MUL);
                        currentCodeUnit.emit(InstructionType::STORE,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        break;
                    case TokenType::SLASH_ASSIGN:
                        currentCodeUnit.emit(InstructionType::LOAD,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        currentCodeUnit.emit(InstructionType::DIV);
                        currentCodeUnit.emit(InstructionType::STORE,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        break;
                    case TokenType::MODULO_ASSIGN:
                        currentCodeUnit.emit(InstructionType::LOAD,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        currentCodeUnit.emit(InstructionType::MOD);
                        currentCodeUnit.emit(InstructionType::STORE,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        break;
                }
            } else if (peek(TokenType::COLON)) {
                eat(TokenType::COLON);
                expression();
                auto op = assignOp();
                currentCodeUnit.emit(InstructionType::DUP);
                expression();
                switch (op) {
                    case TokenType::ASSIGN:
                        currentCodeUnit.emit(InstructionType::LOAD,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        currentCodeUnit.emit(InstructionType::SETIDX);
                        currentCodeUnit.emit(InstructionType::STORE,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        currentCodeUnit.emit(InstructionType::POP);
                        break;
                    case TokenType::PLUS_ASSIGN:
                        currentCodeUnit.emit(InstructionType::LOAD,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        currentCodeUnit.emit(InstructionType::IDX);
                        currentCodeUnit.emit(InstructionType::ADD);
                        currentCodeUnit.emit(InstructionType::LOAD,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        currentCodeUnit.emit(InstructionType::SETIDX);
                        currentCodeUnit.emit(InstructionType::STORE,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        break;
                    case TokenType::MINUS_ASSIGN:
                        currentCodeUnit.emit(InstructionType::LOAD,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        currentCodeUnit.emit(InstructionType::IDX);
                        currentCodeUnit.emit(InstructionType::SUB);
                        currentCodeUnit.emit(InstructionType::LOAD,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        currentCodeUnit.emit(InstructionType::SETIDX);
                        currentCodeUnit.emit(InstructionType::STORE,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        break;
                    case TokenType::STAR_ASSIGN:
                        currentCodeUnit.emit(InstructionType::LOAD,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        currentCodeUnit.emit(InstructionType::IDX);
                        currentCodeUnit.emit(InstructionType::MUL);
                        currentCodeUnit.emit(InstructionType::LOAD,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        currentCodeUnit.emit(InstructionType::SETIDX);
                        currentCodeUnit.emit(InstructionType::STORE,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        break;
                    case TokenType::SLASH_ASSIGN:
                        currentCodeUnit.emit(InstructionType::LOAD,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        currentCodeUnit.emit(InstructionType::IDX);
                        currentCodeUnit.emit(InstructionType::DIV);
                        currentCodeUnit.emit(InstructionType::LOAD,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        currentCodeUnit.emit(InstructionType::SETIDX);
                        currentCodeUnit.emit(InstructionType::STORE,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        break;
                    case TokenType::MODULO_ASSIGN:
                        currentCodeUnit.emit(InstructionType::LOAD,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        currentCodeUnit.emit(InstructionType::IDX);
                        currentCodeUnit.emit(InstructionType::MOD);
                        currentCodeUnit.emit(InstructionType::LOAD,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        currentCodeUnit.emit(InstructionType::SETIDX);
                        currentCodeUnit.emit(InstructionType::STORE,
                                             currentCodeUnit.getConstantIndex(AuroraObj(name)));
                        break;
                }
            } else {
                currentCodeUnit.emit(InstructionType::LOAD,
                                     currentCodeUnit.getConstantIndex(AuroraObj(name)));
                int args;
                if (!peek(TokenType::NEWLINE))
                    args = exprList();
                else
                    args = 0;
                currentCodeUnit.emit(InstructionType::CALL, args);
            }
            eat(TokenType::NEWLINE);
            break;
        }
        case TokenType::NEWLINE:
            eat(TokenType::NEWLINE);
            break;
        default:
            throw AuroraException("Unexpected token " + tokenTypeToString(peek()) + " at line " +
                                  std::to_string(current.line));
    }
}

#define DISPATCH goto *dispatchTable[(int)unit.instructions[++pc].type]

AuroraObj AuroraContext::execute(AuroraCodeUnit unit) {
    unit.emit(InstructionType::END);
    std::stack<AuroraObj> stack{};
    static void *dispatchTable[] = {
            &&PUSH, &&PUSHI, &&TRUE, &&FALSE, &&POP, &&ADD,
            &&SUB, &&MUL, &&DIV, &&MOD, &&NEG, &&NOT,
            &&AND, &&OR, &&EQ, &&NEQ, &&LT, &&GT,
            &&LTE, &&GTE, &&CALL, &&RET, &&RES, &&LOAD,
            &&STORE, &&IF, &&FLOOP, &&WLOOP, &&IDX, &&SETIDX,
            &&BREAK, &&CONTINUE, &&DUP, &&LIST, &&END
    };
    int pc = -1;
    DISPATCH;
    PUSH:
    stack.push(unit.constants[unit.instructions[pc].operand]);
    DISPATCH;
    PUSHI:
    stack.push(AuroraObj((double) unit.instructions[pc].operand));
    DISPATCH;
    TRUE:
    stack.push(AuroraObj(true));
    DISPATCH;
    FALSE:
    stack.push(AuroraObj(false));
    DISPATCH;
    POP:
    stack.pop();
    DISPATCH;
    ADD:
    {
        AuroraObj b = stack.top();
        stack.pop();
        AuroraObj a = stack.top();
        stack.pop();
        if (a.value.index() == 0 && b.value.index() == 0) stack.push(AuroraObj(a.asDouble() + b.asDouble()));
        else if (a.value.index() == 1 && b.value.index() == 1)
            stack.push(AuroraObj(a.asString() + b.asString()));
        else throw AuroraException("Invalid operands for +.");
    }
    DISPATCH;
    SUB:
    {
        AuroraObj b = stack.top();
        stack.pop();
        AuroraObj a = stack.top();
        stack.pop();
        if (a.value.index() == 0 && b.value.index() == 0) stack.push(AuroraObj(a.asDouble() - b.asDouble()));
        else throw AuroraException("Invalid operands for -.");
    }
    DISPATCH;
    MUL:
    {
        AuroraObj b = stack.top();
        stack.pop();
        AuroraObj a = stack.top();
        stack.pop();
        if (a.value.index() == 0 && b.value.index() == 0) stack.push(AuroraObj(a.asDouble() * b.asDouble()));
        else throw AuroraException("Invalid operands for *.");
    }
    DISPATCH;
    DIV:
    {
        AuroraObj b = stack.top();
        stack.pop();
        AuroraObj a = stack.top();
        stack.pop();
        if (a.value.index() == 0 && b.value.index() == 0) stack.push(AuroraObj(a.asDouble() / b.asDouble()));
        else throw AuroraException("Invalid operands for /.");
    }
    DISPATCH;
    MOD:
    {
        AuroraObj b = stack.top();
        stack.pop();
        AuroraObj a = stack.top();
        stack.pop();
        if (a.value.index() == 0 && b.value.index() == 0) stack.push(AuroraObj(dmod(a.asDouble(), b.asDouble())));
        else throw AuroraException("Invalid operands for %.");
    }
    DISPATCH;
    NEG:
    {
        AuroraObj a = stack.top();
        stack.pop();
        if (a.value.index() == 0) stack.push(AuroraObj(-a.asDouble()));
        else throw AuroraException("Invalid operand for -.");
    }
    DISPATCH;
    NOT:
    {
        AuroraObj a = stack.top();
        stack.pop();
        if (a.value.index() == 2) stack.push(AuroraObj(!a.asBool()));
        else throw AuroraException("Invalid operand for !.");
    }
    DISPATCH;
    AND:
    {
        AuroraObj b = stack.top();
        stack.pop();
        AuroraObj a = stack.top();
        stack.pop();
        if (a.value.index() == 2 && b.value.index() == 2)
            stack.push(AuroraObj(a.asBool() && b.asBool()));
        else throw AuroraException("Invalid operands for &&.");
    }
    DISPATCH;
    OR:
    {
        AuroraObj b = stack.top();
        stack.pop();
        AuroraObj a = stack.top();
        stack.pop();
        if (a.value.index() == 2 && b.value.index() == 2)
            stack.push(AuroraObj(a.asBool() || b.asBool()));
        else throw AuroraException("Invalid operands for ||.");
    }
    DISPATCH;
    EQ:
    {
        AuroraObj b = stack.top();
        stack.pop();
        AuroraObj a = stack.top();
        stack.pop();
        stack.push(AuroraObj(a == b));
    }
    DISPATCH;
    NEQ:
    {
        AuroraObj b = stack.top();
        stack.pop();
        AuroraObj a = stack.top();
        stack.pop();
        stack.push(AuroraObj(a != b));
    }
    DISPATCH;
    LT:
    {
        AuroraObj b = stack.top();
        stack.pop();
        AuroraObj a = stack.top();
        stack.pop();
        if (a.value.index() == 0 && b.value.index() == 0) stack.push(AuroraObj(a.asDouble() < b.asDouble()));
        else if (a.value.index() == 1 && b.value.index() == 1)
            stack.push(AuroraObj(a.asString() < b.asString()));
        else throw AuroraException("Invalid operands for <.");
    }
    DISPATCH;
    GT:
    {
        AuroraObj b = stack.top();
        stack.pop();
        AuroraObj a = stack.top();
        stack.pop();
        if (a.value.index() == 0 && b.value.index() == 0) stack.push(AuroraObj(a.asDouble() > b.asDouble()));
        else if (a.value.index() == 1 && b.value.index() == 1)
            stack.push(AuroraObj(a.asString() > b.asString()));
        else throw AuroraException("Invalid operands for >.");
    }
    DISPATCH;
    LTE:
    {
        AuroraObj b = stack.top();
        stack.pop();
        AuroraObj a = stack.top();
        stack.pop();
        if (a.value.index() == 0 && b.value.index() == 0) stack.push(AuroraObj(a.asDouble() <= b.asDouble()));
        else if (a.value.index() == 1 && b.value.index() == 1)
            stack.push(AuroraObj(a.asString() <= b.asString()));
        else throw AuroraException("Invalid operands for <=.");
    }
    DISPATCH;
    GTE:
    {
        AuroraObj b = stack.top();
        stack.pop();
        AuroraObj a = stack.top();
        stack.pop();
        if (a.value.index() == 0 && b.value.index() == 0) stack.push(AuroraObj(a.asDouble() >= b.asDouble()));
        else if (a.value.index() == 1 && b.value.index() == 1)
            stack.push(AuroraObj(a.asString() >= b.asString()));
        else throw AuroraException("Invalid operands for >=.");
    }
    DISPATCH;
    IF:
    {
        AuroraCodeUnit then = stack.top().asCodeUnit();
        stack.pop();
        AuroraCodeUnit else_ = stack.top().asCodeUnit();
        stack.pop();
        AuroraObj cond = stack.top();
        stack.pop();
        if (cond.value.index() == 2) {
            locals.emplace_back();
            if (cond.asBool()) execute(then);
            else execute(else_);
            locals.pop_back();
        } else throw AuroraException("Invalid operand for if.");
    }
    DISPATCH;
    WLOOP:
    {
        AuroraCodeUnit body = stack.top().asCodeUnit();
        stack.pop();
        AuroraCodeUnit cond = stack.top().asCodeUnit();
        stack.pop();
        while (execute(cond).asBool()) {
            try {
                locals.emplace_back();
                execute(body);
                locals.pop_back();
            } catch (InternalAuroraException &e) {
                if (e.type == InternalAuroraException::Type::BREAK) {
                    locals.pop_back();
                    break;
                } else if (e.type == InternalAuroraException::Type::CONTINUE) {
                    locals.pop_back();
                    continue;
                }
            }
        }
    }
    DISPATCH;
    FLOOP:
    {
        AuroraCodeUnit body = stack.top().asCodeUnit();
        stack.pop();
        AuroraObj iter = stack.top();
        stack.pop();
        auto name = unit.constants[unit.instructions[pc].operand].asString();
        if (iter.value.index() == 3) {
            for (auto &i: iter.asVector()) {
                try {
                    locals.emplace_back();
                    locals.back().emplace(name, i);
                    execute(body);
                    locals.pop_back();
                } catch (InternalAuroraException &e) {
                    if (e.type == InternalAuroraException::Type::BREAK) {
                        locals.pop_back();
                        break;
                    } else if (e.type == InternalAuroraException::Type::CONTINUE) {
                        locals.pop_back();
                        continue;
                    }
                }
            }
        } else if (iter.value.index() == 1) {
            for (auto &i: iter.asString()) {
                try {
                    locals.emplace_back();
                    locals.back().emplace(name, AuroraObj(std::string(1, i)));
                    execute(body);
                    locals.pop_back();
                } catch (InternalAuroraException &e) {
                    if (e.type == InternalAuroraException::Type::BREAK) {
                        locals.pop_back();
                        break;
                    } else if (e.type == InternalAuroraException::Type::CONTINUE) {
                        locals.pop_back();
                        continue;
                    }
                }
            }
        } else throw AuroraException("Invalid operand for for.");
    }
    DISPATCH;
    CALL:
    {
        std::vector<AuroraObj> args{};
        for (int i = 0; i < unit.instructions[pc].operand; i++) {
            args.insert(args.begin(), stack.top());
            stack.pop();
        }
        auto fnObj = stack.top();
        stack.pop();
        if (fnObj.value.index() == 4) {
            auto fn = fnObj.asFunction();
            auto tmpLocals = locals;
            locals = {};
            locals.emplace_back();
            for (int i = 0; i < fn.parameters.size(); i++) {
                locals.back().emplace(fn.parameters[i], args[i]);
            }
            AuroraObj retVal;
            try {
                execute(fn.code);
            } catch (AuroraReturnException &e) {
                retVal = e.value;
            }
            stack.push(retVal);
            locals = tmpLocals;
        } else if (fnObj.value.index() == 7) {
            auto fn = fnObj.asNativeFunction();
            stack.push(fn(args));
        } else throw AuroraException("Invalid operand for call.");
    }
    DISPATCH;
    RET:
    throw AuroraReturnException(stack.top());
    RES:
    return stack.top();
    LOAD:
    stack.push(lookupVariable(unit.constants[unit.instructions[pc].operand].asString()));
    DISPATCH;
    STORE:
    setVariable(unit.constants[unit.instructions[pc].operand].asString(), stack.top());
    stack.pop();
    DISPATCH;
    IDX:
    {
        AuroraObj a = stack.top();
        stack.pop();
        AuroraObj b = stack.top();
        stack.pop();
        if (b.value.index() == 3 && a.value.index() == 0) stack.push(b.asVector()[a.asDouble()]);
        else if (b.value.index() == 1 && a.value.index() == 0)
            stack.push(AuroraObj(std::string(1, b.asString()[a.asDouble()])));
        else throw AuroraException("Invalid operands for indexing.");
    }
    DISPATCH;
    SETIDX:
    {
        AuroraObj a = stack.top();
        stack.pop();
        AuroraObj c = stack.top();
        stack.pop();
        AuroraObj b = stack.top();
        stack.pop();
        if (a.value.index() == 3 && b.value.index() == 0) {
            auto newVec = a.asVector();
            newVec.at(b.asDouble()) = c;
            stack.push(AuroraObj(newVec));
        } else if (a.value.index() == 1 && b.value.index() == 0) {
            auto newStr = a.asString();
            newStr.at(b.asDouble()) = c.asString()[0];
            stack.push(AuroraObj(newStr));
        } else throw AuroraException("Invalid operands for indexing.");
    }
    DISPATCH;
    BREAK:
    throw InternalAuroraException(InternalAuroraException::Type::BREAK);
    CONTINUE:
    throw InternalAuroraException(InternalAuroraException::Type::CONTINUE);
    DUP:
    stack.push(stack.top());
    DISPATCH;
    LIST:
    {
        std::vector<AuroraObj> list;
        for (int i = 0; i < unit.instructions[pc].operand; i++) {
            list.push_back(stack.top());
            stack.pop();
        }
        std::reverse(list.begin(), list.end());
        stack.push(AuroraObj(list));
    }
    DISPATCH;
    END:
    return AuroraObj();
}