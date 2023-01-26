//
// Created by snwy on 1/22/23.
//

#ifndef AURORA_INSTRUCTION_H
#define AURORA_INSTRUCTION_H

#include <vector>

enum class InstructionType {
    PUSH,
    PUSHI,
    TRUE,
    FALSE,
    POP,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    NEG,
    NOT,
    AND,
    OR,
    EQ,
    NEQ,
    LT,
    GT,
    LTE,
    GTE,
    CALL,
    RET,
    RES,
    LOAD,
    STORE,
    IF,
    FLOOP,
    WLOOP,
    IDX,
    SETIDX,
    BREAK,
    CONTINUE,
    DUP,
    LIST,
    END
};

struct  __attribute__ ((packed)) Instruction {
    InstructionType type;
    int operand;
};

#endif //AURORA_INSTRUCTION_H
