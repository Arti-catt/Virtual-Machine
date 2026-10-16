#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/file.h>
#include <unistd.h>

#define GONDON(condition, text)     \
    if (!(condition)) {     \
        fprintf(stderr, "File: %s:%d, Function: %s\n%s\n",   \
            __FILE__, __LINE__, __FUNCTION__, text);   \
        exit(true);    \
    }

    enum ErrCodes {
    END = 1,
};

enum Commands {
    PUSH   =   1,
    ADD    =   2,
    SUB    =   3,
    DIV    =   4,
    MUL    =   5,
    OUT    =   6,
    HLT    =   7,
    PUSHR  =   8,
    POP    =   9,
    JUMP   =  10,
};

enum Registers {
    AX = 1,
    BX = 2,
    CX = 3,
    DX = 4,
};

int main()
{
    

    return 0;
}