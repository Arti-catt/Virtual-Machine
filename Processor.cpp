#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/file.h>
#include <unistd.h>

#define STACK_DEBUG
#define PROC_DEBUG

#include "Stack.h"
#define FILENAME "code.txt"
// Processor's DEBUG-system
#ifdef PROC_DEBUG
    #define __OTL__(...) __VA_ARGS__
    #define OTLPRINTSTACK __OTL__(printf("IP = %zd, SIZE = %zd\n", data->ip, data->size);    \
                    printf("REGISTERS: AX = %d, BX = %d, CX = %d, DX = %d\n\n",      \
                        data->ax, data->bx, data->cx, data->dx);    \
                    PrintOTLstack(stk);    \
                    PrintOTLbufline(data);     \
                    printf("YOU ARE HERE: function %s, file %s:%d\n", __FUNCTION__, __FILE__, __LINE__);   \
                    while (getchar() != '\n') {})
#else 
    #define __OTL__(...)
    #define OTLPRINTSTACK
#endif

#define RED "\033[31m"
#define BLUE "\033[35m"
#define GREEN "\033[1;32m"
#define MASLO "\033[1;37m"
#define RES "\033[0m"

#define GONDON(condition, text)     \
    if (!(condition)) {     \
        fprintf(stderr, "File: %s:%d, Function: %s\n%s\n",   \
            __FILE__, __LINE__, __FUNCTION__, text);   \
        exit(true);    \
    }
#define MAXLEN 1000

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
    JMP    =  10,
};

enum Registers {
    AX = 1,
    BX = 2,
    CX = 3,
    DX = 4,
};

struct ComBuff {
    int *bufline;
    ssize_t size;
    ssize_t ip;

    int ax;
    int bx;
    int cx;
    int dx;     //TODO: register's massive
};

void ReadCommands(ComBuff *data);
void WriteInBuf(const int file_ind, char *buffer, const size_t file_size);
ssize_t BufnaBuf_int(int *buf_res, char *buffer);
size_t Fsize(int file_ind);

int CommandExe(ComBuff *data, Stack_t *stk);
// Processor's functions
void Add(Stack_t *stk);
void Sub(Stack_t *stk);
void Mul(Stack_t *stk);
void Div(Stack_t *stk);
void PushRegister(Stack_t *stk, ComBuff *data, int regn);
void PopRegister(Stack_t *stk, ComBuff *data, int regn);
void Jmp(ComBuff *data, int index);

void PrintOTLbufline(ComBuff *data);
void PrintOTLstack(Stack_t *stk);

int main()
{
    Stack_t stk = {};
    StackInit(&stk, 10);

    ComBuff data = {};
    ReadCommands(&data);
    // for (int i = 0; i < data.size; i++) {
        // printf("%d ", data.bufline[i]);
    // }
    // printf("\n");
    for (data.ip = 0; data.ip < data.size; data.ip++) {
        if (CommandExe(&data, &stk) != 0)
            break;
    }

    StackDestroy(&stk);
    return 0;
}

int CommandExe(ComBuff *data, Stack_t *stk) 
{
    GONDON(data, "GONDON HELP YOU")
    GONDON(stk, "GONDON HELP YOU")

    OTLPRINTSTACK
    switch(data->bufline[data->ip]) {
        case PUSH:
            StackPush(stk, data->bufline[++data->ip]);
            break;
        case OUT: {
            int elem = 0;
            __OTL__(printf("PRINT AFTER COMMAND <%sOUT%s>:\n", BLUE, RES);)
            printf("%s%d%s\n", MASLO, elem = StackPop(stk), RES);
            __OTL__(printf("\n");)
            StackPush(stk, elem);
            break;
        }
        case POP:
            PopRegister(stk, data, data->bufline[++data->ip]);
            break;
        case HLT:
            printf("EDRO is TERMINATED\n");
            return END;
        case ADD:
            Add(stk);
            break;
        case SUB:
            Sub(stk);
            break;
        case DIV:
            Div(stk);
            break;
        case MUL:
            Mul(stk);
            break;
        case PUSHR:
            PushRegister(stk, data, data->bufline[++data->ip]);
            break;
        case JMP:
            Jmp(data, data->bufline[++data->ip]);
            break;
        default:
            fprintf(stderr, "ERROR: Command with number %d doesn't exist\n", data->bufline[data->ip]);
            return END;
    }
    OTLPRINTSTACK
    return 0;
}

void Jmp(ComBuff *data, int index)
{
    GONDON(data, "GONDON HELP YOU");
    if (index == -1) {
        fprintf(stderr, "LABEL BELOW ZERO\n");
        abort();
    }
    data->ip = index - 1;
}

void PopRegister(Stack_t *stk, ComBuff *data, int regn)
{
    switch (regn) {
        case AX:
            data->ax = StackPop(stk);
            break;
        case BX:
            data->bx = StackPop(stk);
            break;
        case CX:
            data->cx = StackPop(stk);
            break;
        case DX:
            data->dx = StackPop(stk);
            break;
        default:
            fprintf(stderr, "ERROR: Register with number %d doesn't exist\n", regn);
            exit(true);
    }
}

void PushRegister(Stack_t *stk, ComBuff *data, int regn)
{
    GONDON(stk, "GONDON HELP YOU");

    switch (regn) {
        case AX:
            StackPush(stk, data->ax);
            break;
        case BX:
            StackPush(stk, data->bx);
            break;
        case CX:
            StackPush(stk, data->cx);
            break;
        case DX:
            StackPush(stk, data->dx);
            break;
        default:
            fprintf(stderr, "ERROR: Register with number %d doesn't exist\n", regn);
            exit(true);
    }
}

void Add(Stack_t *stk)
{
    GONDON(stk, "GONDON HELP YOU");
    GONDON(stk->size >= 2, "GONDON HELP YOU");

    ElemType second = StackPop(stk);
    StackPush(stk, StackPop(stk) + second);
}

void Sub(Stack_t *stk)
{
    GONDON(stk, "GONDON HELP YOU");
    GONDON(stk->size >= 2, "GONDON HELP YOU");

    ElemType second = StackPop(stk);
    StackPush(stk, StackPop(stk) - second);
}

void Mul(Stack_t *stk)
{
    GONDON(stk, "GONDON HELP YOU");
    GONDON(stk->size >= 2, "GONDON HELP YOU");

    ElemType second = StackPop(stk);
    StackPush(stk, StackPop(stk) * second);
}

void Div(Stack_t *stk)
{
    GONDON(stk, "GONDON HELP YOU");
    GONDON(stk->size >= 2, "GONDON HELP YOU");

    ElemType second = StackPop(stk);
    GONDON(second, "Zero division, doubloeb");
    StackPush(stk, StackPop(stk) / second);
}

void ReadCommands(ComBuff *data)
{
    GONDON(data != NULL, "GONDON HELP YOU");

    int file_ind = open(FILENAME, O_RDONLY, 0);
    size_t buf_size = Fsize(file_ind);

    char *buffer = NULL;
    GONDON((buffer = (char *) calloc(buf_size, sizeof(char))) != NULL, "GONDON HELP YOU");
    GONDON((data->bufline = (int *) calloc(MAXLEN, sizeof(ssize_t))) != NULL, "GONDON HELP YOU");
    WriteInBuf(file_ind, buffer, buf_size);
    buf_size = BufnaBuf_int(data->bufline, buffer);

    data->size = buf_size;
    close(file_ind);
}

void WriteInBuf(const int file_ind, char *buffer, const size_t file_size)
{
    GONDON(buffer, "GONDON HELP YOU");

    GONDON(read(file_ind, buffer, file_size) > 0, "GONDON HELP YOU");
}
// Read from char buffer to int buffer only
ssize_t BufnaBuf_int(int *buf_res, char *buffer)
{
    GONDON(buf_res, "GONDON HELP YOU");
    GONDON(buffer, "GONDON HELP YOU");

    ssize_t count = 0;
    int fields = 0;
    while (sscanf(buffer, "%d%n", &buf_res[count], &fields) > 0) {
        buffer += fields;
        count++;
    }
    return count;
}

size_t Fsize(int file_ind)
{
    struct stat file_info = {};
    fstat(file_ind, &file_info);
    return file_info.st_size;
}

void PrintOTLbufline(ComBuff *data)
{
    GONDON(data, "GONDON HELP YOU");
    printf(">>MASSIVE OF CODE:\n");
    for (ssize_t i = 0; i < data->size; i++) {
        printf("%6zd\t", i);
    }
    printf("\n");

    for (ssize_t i = 0; i < data->size; i++) {
        if (i == data->ip)
            printf("%s%6d%s\t", GREEN, data->bufline[i], RES);
        else
            printf("%6d\t", data->bufline[i]);
    }
    printf("\n");

    for (ssize_t i = 0; i < data->size; i++) {
        if (i == data->ip)
            printf("%s%6s%s\t", MASLO, "IP", RES);
        else
            printf("%6s\t", "");
    }
    printf("\n\n");
}

void PrintOTLstack(Stack_t *stk)
{
    GONDON(stk, "")
    printf(">>STACK:\n");

    if (stk->size == 0) {
        printf("\nStack is void\n\n");
        return;
    }
    for (ssize_t i = 0; i < stk->size; i++) {
        printf("%6zd\t", i);
    }
    printf("\n");

    for (ssize_t i = 0; i < stk->size; i++) {
        if (i == stk->size - 1)
            printf("%s%6d%s\t", GREEN, stk->stack[i], RES);
        else
            printf("%6d\t", stk->stack[i]);
    }
    printf("\n\n");
}