#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/file.h>
#include <unistd.h>

#define STACK_DEBUG
// #define PROC_DEBUG

#include "Stack.h"
#include "VM_Libs_Enums.h"

#define FILENAME "code.txt"
// Processor's DEBUG-system
#ifdef PROC_DEBUG
    #define __OTL__(...) __VA_ARGS__
    #define OTLPRINTSTACK __OTL__(printf("IP = %zd, SIZE = %zd\n", data->ip, data->size);                   \
                    printf("REGISTERS: AX = %d, BX = %d, CX = %d, DX = %d\n\n",                             \
                        data->ax, data->bx, data->cx, data->dx);                                            \
                    PrintOTLstack(frame, "FRAME");                                                          \
                    PrintOTLstack(stk, "STACK");                                                            \
                    PrintOTLbufline(data);                                                                  \
                    printf("YOU ARE HERE: function %s, file %s:%d\n", __FUNCTION__, __FILE__, __LINE__);    \
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

#define GONDON(condition, text)                                 \
    if (!(condition)) {                                         \
        fprintf(stderr, "File: %s:%d, Function: %s\n%s\n",      \
            __FILE__, __LINE__, __FUNCTION__, text);            \
        exit(true);                                             \
    }

#define MAXLEN 1000

enum ErrCodes {
    END = 1,
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
void GetOneCoef(int *coef);

int CommandExe(ComBuff *data, Stack_t *stk, Stack_t *frame);
// PROCESSOR'S FUNCTIONS
void Add(Stack_t *stk);
void Sub(Stack_t *stk);
void Mul(Stack_t *stk);
void Div(Stack_t *stk);
void PushRegister(Stack_t *stk, ComBuff *data, int regn);
void PopRegister(Stack_t *stk, ComBuff *data, int regn);
void Jmp(ComBuff *data, int label);
void CondJA(Stack_t *stk, ComBuff *data, int label);
void CondJAE(Stack_t *stk, ComBuff *data, int label);
void CondJB(Stack_t *stk, ComBuff *data, int label);
void CondJBE(Stack_t *stk, ComBuff *data, int label);
void CondJE(Stack_t *stk, ComBuff *data, int label);
void CondJNE(Stack_t *stk, ComBuff *data, int label);
void FuncIN(Stack_t *stk, ComBuff *data);
void Call(Stack_t *stk, ComBuff *data, Stack_t *frame, int label);
void Ret(Stack_t *stk, ComBuff *data, Stack_t *frame);


void PrintOTLbufline(ComBuff *data);
void PrintOTLstack(Stack_t *stk, const char *name);

int main()
{
    Stack_t stk = {};
    StackInit(&stk, 10);

    Stack_t frame = {};
    StackInit(&frame, 10);

    ComBuff data = {};
    ReadCommands(&data);
    // for (int i = 0; i < data.size; i++) {
        // printf("%d ", data.bufline[i]);
    // }
    // printf("\n");
    for (data.ip = 0; data.ip < data.size; data.ip++) {
        if (CommandExe(&data, &stk, &frame) != 0)
            break;
    }
    StackDestroy(&stk);
    return 0;
}

int CommandExe(ComBuff *data, Stack_t *stk, Stack_t *frame) 
{
    GONDON(data, "GONDON HELP YOU")
    GONDON(stk, "GONDON HELP YOU")

    OTLPRINTSTACK
    switch(data->bufline[data->ip]) {
        case PUSH:
            // printf("CAN BEFORE: %d\n", stk->real_stack[0]);
            StackPush(stk, data->bufline[++data->ip]);
            // printf("CAN AFTER: %d\n", stk->real_stack[0]);
            // printf("buf[IP] = %d\n", data->bufline[data->ip]);
            break;
        case OUT: {
            int elem = 0;
            __OTL__(printf("PRINT AFTER COMMAND <%sOUT%s>:\n", BLUE, RES);)
            printf("%d\n", elem = StackPop(stk));
            __OTL__(printf("\n");)
            StackPush(stk, elem);
            break;
        }
        case POPR:
            PopRegister(stk, data, data->bufline[++data->ip]);
            break;
        case HLT:
            printf("EDRO is %sTERMINATED%s\n", MASLO, RES);
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
        case JA:
            CondJA(stk, data, data->bufline[++data->ip]);
            break;
        case JAE:
            CondJAE(stk, data, data->bufline[++data->ip]);
            break;
        case JB:
            CondJB(stk, data, data->bufline[++data->ip]);
            break;
        case JBE:
            CondJBE(stk, data, data->bufline[++data->ip]);
            break;
        case JE:
            CondJE(stk, data, data->bufline[++data->ip]);
            break;
        case JNE:
            CondJNE(stk, data, data->bufline[++data->ip]);
            break;
        case IN:
            FuncIN(stk, data);
            break;
        case CALL:
            Call(stk, data, frame, data->bufline[++data->ip]);
            break;
        case RET:
            Ret(stk, data, frame);
            break;
        default:
            fprintf(stderr, "ERROR: Command with number %d doesn't exist\n", data->bufline[data->ip]);
            return END;
    }
    OTLPRINTSTACK
    return 0;
}

void Ret(Stack_t *stk, ComBuff *data, Stack_t *frame)
{
    GONDON(stk, "GONDON HELP YOU")
    GONDON(data, "GONDON HELP YOU")
    GONDON(frame, "GONDON HELP YOU")

    data->ip = StackPop(frame) - 1;
}

void Call(Stack_t *stk, ComBuff *data, Stack_t *frame, int label)
{
    GONDON(stk, "GONDON HELP YOU")
    GONDON(data, "GONDON HELP YOU")
    GONDON(frame, "GONDON HELP YOU")

    StackPush(frame, data->ip + 1);

    if (label == -1) {
        fprintf(stderr, "LABEL BELOW ZERO\n");
        exit(true);
    }
    data->ip = label - 1;
}

void FuncIN(Stack_t *stk, ComBuff *data)
{
    GONDON(stk, "GONDON HELP YOU")
    GONDON(data, "GONDON HELP YOU")
    
    int number = 0;
    GetOneCoef(&number);
    StackPush(stk, number);
}

void GetOneCoef(int *coef) 
{
    GONDON(coef, "GONDON HELP YOU");

    int numb_fields = 0;
    __OTL__(printf("Enter number: ");)
    GONDON(numb_fields = scanf("%d", coef), "GONDON HELP YOU")
}

void CondJA(Stack_t *stk, ComBuff *data, int label)
{
    GONDON(stk, "GONDON HELP YOU")
    GONDON(data, "GONDON HELP YOU")
    GONDON(stk->size >= 2, "GONDON HELP YOU")

    int elem2 = StackPop(stk);
    int elem1 = StackPop(stk);
    if (elem1 > elem2)
        Jmp(data, label);
    // printf("IP = %zd\n", data->ip);
    StackPush(stk, elem1);
    StackPush(stk, elem2);
}

void CondJAE(Stack_t *stk, ComBuff *data, int label)
{
    GONDON(stk, "GONDON HELP YOU")
    GONDON(data, "GONDON HELP YOU")

    int elem2 = StackPop(stk);
    int elem1 = StackPop(stk);

    if (elem1 >= elem2)
        Jmp(data, label);
}

void CondJB(Stack_t *stk, ComBuff *data, int label)
{
    GONDON(stk, "GONDON HELP YOU")
    GONDON(data, "GONDON HELP YOU")

    int elem2 = StackPop(stk);
    int elem1 = StackPop(stk);

    if (elem1 < elem2)
        Jmp(data, label);
}

void CondJBE(Stack_t *stk, ComBuff *data, int label)
{
    GONDON(stk, "GONDON HELP YOU")
    GONDON(data, "GONDON HELP YOU")

    int elem2 = StackPop(stk);
    int elem1 = StackPop(stk);

    if (elem1 <= elem2)
        Jmp(data, label);
}

void CondJE(Stack_t *stk, ComBuff *data, int label)
{
    GONDON(stk, "GONDON HELP YOU")
    GONDON(data, "GONDON HELP YOU")

    int elem2 = StackPop(stk);
    int elem1 = StackPop(stk);

    if (elem1 == elem2)
        Jmp(data, label);
}

void CondJNE(Stack_t *stk, ComBuff *data, int label)
{
    GONDON(stk, "GONDON HELP YOU")
    GONDON(data, "GONDON HELP YOU")

    int elem2 = StackPop(stk);
    int elem1 = StackPop(stk);

    if (elem1 != elem2)
        Jmp(data, label);
}

void Jmp(ComBuff *data, int label)
{
    GONDON(data, "GONDON HELP YOU");
    if (label == -1) {
        fprintf(stderr, "LABEL BELOW ZERO\n");
        abort();
    }
    data->ip = label - 1;
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

    StackPush(stk, StackPop(stk) + StackPop(stk));
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

    StackPush(stk, StackPop(stk) * StackPop(stk));
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

void PrintOTLstack(Stack_t *stk, const char *name)
{
    GONDON(stk, "")
    printf(">>%s:\n", name);

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