//
// Created by snwy on 1/22/23.
//

#ifndef AURORA_LEXER_H
#define AURORA_LEXER_H

#include <string>
#include <any>
#include <unordered_map>

enum class TokenType {
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, MINUS, PLUS, SLASH, STAR, MODULO,
    LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
    EQUAL, NOT_EQUAL, AND, OR, NOT, COLON,
    IDENTIFIER, STRING, NUMBER, TRUE, FALSE, NIL,
    IF, ELSE, WHILE, FOR, FN, RETURN, BREAK, CONTINUE,
    NEWLINE, END, ASSIGN, PLUS_ASSIGN, MINUS_ASSIGN, STAR_ASSIGN, SLASH_ASSIGN, MODULO_ASSIGN,
    ARROW, EOF_,
};

struct Token {
    TokenType type;
    std::string lexeme;
    std::any literal;
    int line;

    Token(TokenType type, std::string lexeme, std::any literal, int line)
            : type(type), lexeme(std::move(lexeme)), literal(std::move(literal)), line(line) {}
};

class Lexer {
private:
    std::string source;
    int line = 1;
    int current = 0;

    char advance();

    char peek();

    bool match(char expected);

    std::unordered_map<std::string, TokenType> keywords = {
            {"and",      TokenType::AND},
            {"or",       TokenType::OR},
            {"not",      TokenType::NOT},
            {"if",       TokenType::IF},
            {"else",     TokenType::ELSE},
            {"while",    TokenType::WHILE},
            {"for",      TokenType::FOR},
            {"fn",       TokenType::FN},
            {"return",   TokenType::RETURN},
            {"break",    TokenType::BREAK},
            {"continue", TokenType::CONTINUE},
            {"true",     TokenType::TRUE},
            {"false",    TokenType::FALSE},
            {"nil",      TokenType::NIL},
            {"end",      TokenType::END},
    };

    static bool isAlpha(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '?';
    }

    Token string();

    Token number();

    Token identifier();

public:
    explicit Lexer(std::string source) : source(std::move(source)) {}
    Token nextToken();

    char peekNext();
};

#endif //AURORA_LEXER_H
