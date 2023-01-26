//
// Created by snwy on 1/22/23.
//

#include "lexer.h"
#include "aurora_exception.h"

char Lexer::advance() {
    if (current >= source.length()) return '\0';
    return source[current++];
}

char Lexer::peek() {
    if (current >= source.length()) return '\0';
    return source[current];
}

char Lexer::peekNext() {
    if (current + 1 >= source.length()) return '\0';
    return source[current + 1];
}

bool Lexer::match(char expected) {
    if (source[current] != expected || current >= source.length()) return false;
    current++;
    return true;
}

Token Lexer::string() {
    std::string value;
    while (peek() != '"' && current < source.length()) {
        if (peek() == '\n') line++;
        if (peek() == '\\' && peekNext() == '"') {
            advance();
            advance();
            value += '"';
            continue;
        }
        value += advance();
    }
    if (current >= source.length())
        throw AuroraException("Unterminated string.");
    advance();
    // handle escape sequences
    std::string escaped;
    for (int i = 0; i < value.length(); i++) {
        if (value[i] == '\\' && i + 1 < value.length()) {
            switch (value[i + 1]) {
                case 'n':
                    escaped += '\n';
                    break;
                case 't':
                    escaped += '\t';
                    break;
                case 'r':
                    escaped += '\r';
                    break;
                case 'b':
                    escaped += '\b';
                    break;
                case 'f':
                    escaped += '\f';
                    break;
                case 'v':
                    escaped += '\v';
                    break;
                case 'a':
                    escaped += '\a';
                    break;
                case '\\':
                    escaped += '\\';
                    break;
                case '0':
                    escaped += '\0';
                    break;
                default:
                    escaped += value[i + 1];
                    break;
            }
            i++;
        } else {
            escaped += value[i];
        }
    }
    return {TokenType::STRING, value, value, line};
}

Token Lexer::number() {
    std::string value;
    while (isdigit(peek())) value += advance();
    if (peek() == '.' && isdigit(peekNext())) {
        value += advance();
        while (isdigit(peek())) value += advance();
    }
    return {TokenType::NUMBER, value, std::stod(value), line};
}

Token Lexer::identifier() {
    std::string value;
    while (isAlpha(peek()) || isdigit(peek())) value += advance();
    if (keywords.count(value)) return {keywords[value], value, std::any(), line};
    return {TokenType::IDENTIFIER, value, std::any(), line};
}

Token Lexer::nextToken()  {
    char c = advance();
    switch (c) {
        case '(':
            return {TokenType::LEFT_PAREN, "(", nullptr, line};
        case ')':
            return {TokenType::RIGHT_PAREN, ")", nullptr, line};
        case '{':
            return {TokenType::LEFT_BRACE, "{", nullptr, line};
        case '}':
            return {TokenType::RIGHT_BRACE, "}", nullptr, line};
        case ',':
            return {TokenType::COMMA, ",", nullptr, line};
        case '-':
            if (match('>')) return {TokenType::ARROW, "->", nullptr, line};
            else if (match('=')) return {TokenType::MINUS_ASSIGN, "-=", nullptr, line};
            else return {TokenType::MINUS, "-", nullptr, line};
        case '+':
            if (match('=')) return {TokenType::PLUS_ASSIGN, "+=", nullptr, line};
            else return {TokenType::PLUS, "+", nullptr, line};
        case '/':
            if (match('=')) return {TokenType::SLASH_ASSIGN, "/=", nullptr, line};
            else return {TokenType::SLASH, "/", nullptr, line};
        case '*':
            if (match('=')) return {TokenType::STAR_ASSIGN, "*=", nullptr, line};
            else return {TokenType::STAR, "*", nullptr, line};
        case '%':
            if (match('=')) return {TokenType::MODULO_ASSIGN, "%=", nullptr, line};
            else return {TokenType::MODULO, "%", nullptr, line};
        case '<':
            if (match('=')) return {TokenType::LESS_EQUAL, "<=", nullptr, line};
            else return {TokenType::LESS, "<", nullptr, line};
        case '>':
            if (match('=')) return {TokenType::GREATER_EQUAL, ">=", nullptr, line};
            else return {TokenType::GREATER, ">", nullptr, line};
        case '=':
            if (match('=')) return {TokenType::EQUAL, "==", nullptr, line};
            else return {TokenType::ASSIGN, "=", nullptr, line};
        case ':':
            return {TokenType::COLON, ":", nullptr, line};
        case '\n':
            line++;
            return {TokenType::NEWLINE, "\n", nullptr, line};
        case ' ':
        case '\r':
        case '\t':
            return nextToken();
        case '"':
            return string();
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            current--;
            return number();
        case '\0':
            return {TokenType::EOF_, "", nullptr, line};
        default:
            if (isAlpha(c)) {
                current--;
                return identifier();
            }
            else throw AuroraException("Unexpected character " + std::string(1, c));
    }
}