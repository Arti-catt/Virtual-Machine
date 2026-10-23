#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/file.h>
#include <unistd.h>
#include <malloc.h>

#define NUMBER_COMMANDS 19

enum Commands {
    PUSH    =  1,
    ADD     =  2,
    SUB     =  3,
    DIV     =  4,
    MUL     =  5,
    OUT     =  6,
    HLT     =  7,
    PUSHR   =  8,
    POPR    =  9,
    JMP     = 10,
    JA      = 11,
    JAE     = 12,
    JB      = 13,
    JBE     = 14,
    JE      = 15,
    JNE     = 16,
    IN      = 17,
    CALL    = 18,
    RET     = 19,
};

enum Registers {
    AX = 1,
    BX = 2,
    CX = 3,
    DX = 4,
};