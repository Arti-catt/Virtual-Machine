#include "VM_Libs_Enums.h"

#define STACK_DEBUG
// #define PROC_DEBUG

#include "Stack.h"

#define FILE_NAME "code.txt"
// Processor's DEBUG-system
#ifdef PROC_DEBUG
    #define __OTL__(...) __VA_ARGS__
    #define OTLPRINTSTACK __OTL__(printf("IP = %zd, SIZE = %zd\n", data->ip, data->size);                   \
                    printf("REGISTERS: AX = %lg, BX = %lg, CX = %lg, DX = %lg\n\n",                         \
                        data->registers[0], data->registers[1], data->registers[2], data->registers[3]);    \
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

#define GONDON(condition)                                                   \
    if (!(condition)) {                                                     \
        fprintf(stderr, "File: %s:%d, Function: %s\nGONDON HELP YOU\n",     \
            __FILE__, __LINE__, __FUNCTION__);                              \
        abort();                                                            \
    }

#define MAX_LEN_BUF 1000

enum ErrCodes {
    END = 1,
};

struct ComData {
    ParType *bufline;
    ssize_t size;
    ssize_t ip;

    ParType registers[NUMBER_REGISTERS] = {};
};

void ReadCommands(ComData *data);
void WriteInBuf(const int file_ind, char *buffer, const size_t file_size);
size_t ConvertBuf_ParType(ParType *buf_res, char *buffer);
size_t Fsize(int file_ind);
void GetOneCoef(ParType *coef);

int Processing(ComData *data, Stack_t *stk, Stack_t *frame);
int CommandExe(ComData *data, Stack_t *stk, Stack_t *frame);
// PROCESSOR'S FUNCTIONS
void Add(Stack_t *stk);
void Sub(Stack_t *stk);
void Mul(Stack_t *stk);
void Div(Stack_t *stk);
void Out(Stack_t *stk);
void PushRegister(Stack_t *stk, ComData *data);
void PopRegister(Stack_t *stk, ComData *data);
void Jmp(ComData *data);
void CondJA(Stack_t *stk, ComData *data);
void CondJAE(Stack_t *stk, ComData *data);
void CondJB(Stack_t *stk, ComData *data);
void CondJBE(Stack_t *stk, ComData *data);
void CondJE(Stack_t *stk, ComData *data);
void CondJNE(Stack_t *stk, ComData *data);
void FuncIN(Stack_t *stk, ComData *data);
void Call(Stack_t *stk, ComData *data, Stack_t *frame);
void Ret(Stack_t *stk, ComData *data, Stack_t *frame);


void PrintOTLbufline(ComData *data);
void PrintOTLstack(Stack_t *stk, const char *name);

int main()
{
    Stack_t stk = {};
    StackInit(&stk, 10);

    Stack_t frame = {};
    StackInit(&frame, 10);

    ComData data = {};
    ReadCommands(&data);
    // for (int i = 0; i < data.size; i++) {
        // printf("%d ", data.bufline[i]);
    // }
    // printf("\n");
    Processing(&data, &stk, &frame);

    StackDestroy(&stk);
    return 0;
}

int Processing(ComData *data, Stack_t *stk, Stack_t *frame)
{
    GONDON(data)
    GONDON(stk)
    GONDON(frame)

    int err = 0;
    for (data->ip = 0; err == 0 && data->ip < data->size; data->ip++) {
        err = CommandExe(data, stk, frame);
    }
    return err;
}

int CommandExe(ComData *data, Stack_t *stk, Stack_t *frame) 
{
    GONDON(data)
    GONDON(stk)
    GONDON(frame)

    OTLPRINTSTACK
    switch((int) data->bufline[data->ip]) {
        case PUSH:
            // printf("CAN BEFORE: %d\n", stk->real_stack[0]);
            StackPush(stk, data->bufline[++data->ip]);
            // printf("CAN AFTER: %d\n", stk->real_stack[0]);
            // printf("buf[IP] = %d\n", data->bufline[data->ip]);
            break;
        case OUT: {
            Out(stk);
            break;
        }
        case POPR:
            PopRegister(stk, data);
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
            PushRegister(stk, data);
            break;
        case JMP:
            Jmp(data);
            break;
        case JA:
            CondJA(stk, data);
            break;
        case JAE:
            CondJAE(stk, data);
            break;
        case JB:
            CondJB(stk, data);
            break;
        case JBE:
            CondJBE(stk, data);
            break;
        case JE:
            CondJE(stk, data);
            break;
        case JNE:
            CondJNE(stk, data);
            break;
        case IN:
            FuncIN(stk, data);
            break;
        case CALL:
            Call(stk, data, frame);
            break;
        case RET:
            Ret(stk, data, frame);
            break;
        default:
            fprintf(stderr, "ERROR: Command with number %lg doesn't exist\n", data->bufline[data->ip]);
            return END;
    }
    OTLPRINTSTACK
    return 0;
}

void Ret(Stack_t *stk, ComData *data, Stack_t *frame)
{
    GONDON(stk)
    GONDON(data)
    GONDON(frame)

    data->ip = (ssize_t) StackPop(frame) - 1;
}

void Call(Stack_t *stk, ComData *data, Stack_t *frame)
{
    GONDON(stk)
    GONDON(data)
    GONDON(frame)
    
    int label = (int) data->bufline[++data->ip];
    StackPush(frame, data->ip + 1);

    if (label == -1) {
        fprintf(stderr, "LABEL BELOW ZERO\n");
        exit(true);
    }
    data->ip = label - 1;
}

void FuncIN(Stack_t *stk, ComData *data)
{
    GONDON(stk)
    GONDON(data)
    
    ParType number = 0;
    GetOneCoef(&number);
    StackPush(stk, number);
}

void CondJA(Stack_t *stk, ComData *data)
{
    GONDON(stk)
    GONDON(data)

    ParType elem2 = StackPop(stk);
    ParType elem1 = StackPop(stk);
    if (elem1 > elem2)
        Jmp(data);
    else
        data->ip++;
}

void CondJAE(Stack_t *stk, ComData *data)
{
    GONDON(stk)
    GONDON(data)

    ParType elem2 = StackPop(stk);
    ParType elem1 = StackPop(stk);
    if (elem1 >= elem2)
        Jmp(data);
    else
        data->ip++;
}

void CondJB(Stack_t *stk, ComData *data)
{
    GONDON(stk)
    GONDON(data)

    ParType elem2 = StackPop(stk);
    ParType elem1 = StackPop(stk);

    if (elem1 < elem2)
        Jmp(data);
    else
        data->ip++;
}

void CondJBE(Stack_t *stk, ComData *data)
{
    GONDON(stk)
    GONDON(data)

    ParType elem2 = StackPop(stk);
    ParType elem1 = StackPop(stk);

    if (elem1 <= elem2)
        Jmp(data);
    else
        data->ip++;
}

void CondJE(Stack_t *stk, ComData *data)
{
    GONDON(stk)
    GONDON(data)

    ParType elem2 = StackPop(stk);
    ParType elem1 = StackPop(stk);

    if (elem1 == elem2)
        Jmp(data);
    else
        data->ip++;
}

void CondJNE(Stack_t *stk, ComData *data)
{
    GONDON(stk)
    GONDON(data)

    ParType elem2 = StackPop(stk);
    ParType elem1 = StackPop(stk);

    if (elem1 != elem2)
        Jmp(data);
    else
        data->ip++;
}

void Jmp(ComData *data)
{
    GONDON(data);
    int label = (int) data->bufline[++data->ip];

    if (label == -1) {
        fprintf(stderr, "LABEL BELOW ZERO\n");
        GONDON(false)
    }
    data->ip = label - 1;
}

void PopRegister(Stack_t *stk, ComData *data)
{
    GONDON(stk)
    GONDON(data)
    int regn = (int) data->bufline[++data->ip];

    if (regn < 0 || regn > NUMBER_REGISTERS) {
        fprintf(stderr, "ERROR: Register with number %d doesn't exist\n", regn);
        exit(true);
    }
    data->registers[regn - 1] = StackPop(stk);
}

void PushRegister(Stack_t *stk, ComData *data)
{
    GONDON(stk)
    GONDON(data)
    int regn = (int) data->bufline[++data->ip];

    if (regn <= 0 || regn > NUMBER_REGISTERS) {
        fprintf(stderr, "ERROR: Register with number %d doesn't exist\n", regn);
        exit(true);
    }
    StackPush(stk, data->registers[regn - 1]);
}

void Add(Stack_t *stk)
{
    GONDON(stk)
    GONDON(stk->size >= 2)

    StackPush(stk, StackPop(stk) + StackPop(stk));
}

void Sub(Stack_t *stk)
{
    GONDON(stk)
    GONDON(stk->size >= 2)

    ElemType second = StackPop(stk);
    StackPush(stk, StackPop(stk) - second);
}

void Mul(Stack_t *stk)
{
    GONDON(stk);
    GONDON(stk->size >= 2);

    StackPush(stk, StackPop(stk) * StackPop(stk));
}

void Div(Stack_t *stk)
{
    GONDON(stk);
    GONDON(stk->size >= 2);

    ElemType second = StackPop(stk);
    GONDON(second);
    StackPush(stk, StackPop(stk) / second);
}

void Out(Stack_t *stk)
{
    GONDON(stk)
    double elem = 0;
    __OTL__(printf("PRINT AFTER COMMAND <%sOUT%s>:\n", BLUE, RES);)
    printf("%lg\n", elem = StackPop(stk));
    __OTL__(printf("\n");)
    StackPush(stk, elem);
}

void ReadCommands(ComData *data)
{
    GONDON(data != NULL);

    int file_ind = open(FILE_NAME, O_RDONLY, 0);
    size_t buf_size = Fsize(file_ind);

    char *buffer = NULL;
    buffer = (char *) calloc(buf_size, sizeof(char));
    GONDON(buffer)
    data->bufline = (double *) calloc(MAX_LEN_BUF, sizeof(ssize_t));
    GONDON(data->bufline)

    WriteInBuf(file_ind, buffer, buf_size);
    buf_size = ConvertBuf_ParType(data->bufline, buffer);

    data->size = buf_size;
    close(file_ind);
}

void WriteInBuf(const int file_ind, char *buffer, const size_t file_size)
{
    GONDON(buffer);

    int fields = read(file_ind, buffer, file_size);
    GONDON(fields)
}
// Read from char buffer to int buffer only
size_t ConvertBuf_ParType(ParType *buf_res, char *buffer)
{
    GONDON(buf_res);
    GONDON(buffer);

    size_t count = 0;
    int fields = 0;
    while (sscanf(buffer, "%lg%n", &buf_res[count], &fields) > 0) {
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

void GetOneCoef(ParType *coef) 
{
    GONDON(coef);

    int numb_fields = 0;
    __OTL__(printf("Enter number: ");)
    numb_fields = scanf("%lg", coef);
    GONDON(numb_fields)
}

void PrintOTLbufline(ComData *data)
{
    GONDON(data);
    printf(">>MASSIVE OF CODE:\n");
    for (ssize_t i = 0; i < data->size; i++) {
        printf("%6zd\t", i);
    }
    printf("\n");

    for (ssize_t i = 0; i < data->size; i++) {
        if (i == data->ip)
            printf("%s%6lg%s\t", GREEN, data->bufline[i], RES);
        else
            printf("%6lg\t", data->bufline[i]);
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
    GONDON(stk)
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
            printf("%s%6lg%s\t", GREEN, stk->stack[i], RES);
        else
            printf("%6lg\t", stk->stack[i]);
    }
    printf("\n\n");
}