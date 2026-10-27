typedef double ElemType;

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//                          ТЫ ЗНАЕШЬ СИ. СИ ПОКАЖЕШЬ?
//DEBUG macro system
#ifdef STACK_DEBUG
    #define DEBUG(...) __VA_ARGS__
    #define DEBUG_STACK_DUMP(parameters) StackDump(parameters);
    #define DEBUG_PRINT(...) fprintf(log_file, __VA_ARGS__);
    #define DEBUG_ARGS , __FILE__, __LINE__, __FUNCTION__
    #define DEBUG_PAR , const char *file, int line, const char *function
    #define STACK_VERIFY_ARGS file, line, __FUNCTION__

    //#define LOGFILE "DEBUGlog.txt"
    //FILE *log_file = NULL;
#else
    #define DEBUG(...)
    #define DEBUG_STACK_DUMP(...)
    #define DEBUG_PRINT(...)
    #define DEBUG_ARGS
    #define DEBUG_PAR
    #define STACK_VERIFY_ARGS __FILE__, __LINE__, __FUNCTION__
#endif
//Function macro-names for connecting with DEBUG system
#define StackInit(stk, capacity) _StackInit_(stk, capacity DEBUG_ARGS)
#define StackPush(stk, elem) _StackPush_(stk, elem DEBUG_ARGS)
#define StackPop(stk) _StackPop_(stk DEBUG_ARGS)
#define StackDestroy(stk) _StackDestroy_(stk DEBUG_ARGS)
#define StackPrint(stk) _StackPrint_(stk DEBUG_ARGS)

#define CANARY 0xD0BB1
#define STK_CANARY 0xEB1D0EB1

struct Stack_t {
    ElemType canary_t1 = CANARY;
    ElemType *real_stack;
	ElemType *stack;
    ssize_t size;
    ssize_t capacity;
    ElemType canary_t2 = CANARY;
};

void _StackInit_(Stack_t *stk, ssize_t capacity DEBUG_PAR);
void _StackPush_(Stack_t *stk, ElemType elem DEBUG_PAR);
ElemType _StackPop_(Stack_t *stk DEBUG_PAR);

void _StackDestroy_(Stack_t *stk DEBUG_PAR);

void _StackPrint_(Stack_t *stk DEBUG_PAR);
