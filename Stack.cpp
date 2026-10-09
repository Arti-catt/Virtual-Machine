typedef int StackElem_t;

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//#define STACK_DEBUG       //TURN DEBUG ON/OFF
//                          ТЫ ЗНАЕШЬ СИ. СИ ПОКАЖЕШЬ?
//DEBUG macro system
#ifdef STACK_DEBUG
    #define DEBUG(...) __VA_ARGS__
    #define DEBUG_STACK_DUMP(parametres) StackDump(parametres);
    #define DEBUG_PRINT(...) fprintf(log_file, __VA_ARGS__);
    #define DEBUG_ARGS , __FILE__, __LINE__, __FUNCTION__
    #define DEBUG_PAR , const char *file, int line, const char *function
    #define STACK_VERIFY_ARGS file, line, __FUNCTION__

    #define LOGFILE "DEBUGlog.txt"
    FILE *log_file = NULL;
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
//PRINTERR made for NON_FATAL errors
#define PRINTERR(text) fprintf(stderr, "%s\n", text);   \
                        DEBUG_PRINT("%s\n", text);
//GONDON_STK can be used in function with poiner on struct <stk> ONLY
#define GONDON_STK(condition, text)     \
    if (!(condition)) {     \
        fprintf(stderr, "File: %s:%d, Function: %s\n%s\n",   \
            STACK_VERIFY_ARGS, text);   \
        DEBUG_PRINT(">>GONDON_STK<< File: %s:%d, Function: %s\n%s\n",   \
            STACK_VERIFY_ARGS, text);      \
        DEBUG_STACK_DUMP(stk)   \
        exit(true);    \
    }
//macro STACK_OK check ФАТАЛЬНОСТЬ error-a got from function Stack_Verify
#define STACK_OK GONDON_STK(Stack_Verify(stk, STACK_VERIFY_ARGS) != FATAL_ERR,   \
                                "ERROR: STRUCT <Stack_t> HAVE A PROBLEM, HOUSTON")
#define VOID -777
#define CANARY 0xD0BB1
#define STK_CANARY 0xEB1D0EB1

enum ErrorCodes {
    ERRNO          =  0,
    NON_FATAL_ERR  =  1,
    FATAL_ERR      =  2,
};

enum ReallocMods {
    DOWN  =  0,
    UP    =  1,
};

struct Stack_t {
    StackElem_t canary_t1 = CANARY;
    StackElem_t *real_stack;
	StackElem_t *stack;
    ssize_t size;
    ssize_t capacity;
    StackElem_t canary_t2 = CANARY;
};

void _StackInit_(Stack_t *stk, ssize_t capacity DEBUG_PAR);
void _StackPush_(Stack_t *stk, StackElem_t elem DEBUG_PAR);
StackElem_t _StackPop_(Stack_t *stk DEBUG_PAR);
StackElem_t *ReSize(Stack_t *stk, int mod);

void _StackDestroy_(Stack_t *stk DEBUG_PAR);

void _StackPrint_(Stack_t *stk DEBUG_PAR);

void StackDump(Stack_t *stk);
uint Stack_Verify(Stack_t *stk,
            const char *file, int line, const char *function);

uint Stack_Verify(Stack_t *stk,
            const char *file, int line, const char *function)
{
    DEBUG_PRINT("Stack_Verify protocol:\n"
                "File: %s:%d, Function: %s\n", file, line, function);

    int err = ERRNO;
    if (stk == NULL) {
        PRINTERR("WARNING_FATAL: pointer on struct <Stack_t> is NULL")
        return FATAL_ERR;
    }
    if (stk->canary_t1 != CANARY) {
        PRINTERR("WARNING_FATAL: LEFT canary have CHANGED")
        err = FATAL_ERR;
    }
    if (stk->canary_t2 != CANARY) {
        PRINTERR("WARNING_FATAL: RIGHT canary have CHANGED")
        err = FATAL_ERR;
    }
    if (stk->size < 0) {
        PRINTERR("WARNING_FATAL: size < 0 in struct <Stack_t>")
        err = FATAL_ERR;
    }
    if (stk->capacity < 0) {
        PRINTERR("WARNING_FATAL: capacity < 0 in struct <Stack_t>")
        err = FATAL_ERR;
    }
    if (stk->size > stk->capacity) {
        PRINTERR("WARNING_FATAL: size > capacity in struct <Stack_t>")
        err = FATAL_ERR;
    }
    if (stk->real_stack == NULL) {
        PRINTERR("WARNING_FATAL: pointer on real_stack in struct <Stack_t> is NULL")
        return FATAL_ERR;
    }
    if (stk->stack == NULL) {
        PRINTERR("WARNING_FATAL: pointer on stack in struct <Stack_t> is NULL")
        return FATAL_ERR;
    }
    if (stk->real_stack[0] != (StackElem_t) STK_CANARY) {
        PRINTERR("WARNING: LEFT stack canary have changed")
        (err == FATAL_ERR) ? : err = NON_FATAL_ERR;
    }
    if (stk->real_stack[stk->capacity + 1] != (StackElem_t) STK_CANARY) {
        PRINTERR("WARNING: RIGHT stack canary have changed")
        (err == FATAL_ERR) ? : err = NON_FATAL_ERR;
    }

    DEBUG(switch (err) {
        case ERRNO:
            DEBUG_PRINT("Stack_Verify didn't detect errors\n");
            break;
        case NON_FATAL_ERR:
            DEBUG_PRINT("Stack_Verify DETECTED NON_FATAL_ERRORS\n");
            break;
        default:
            DEBUG_PRINT("Stack_Verify DETECTED FATAL_ERRORS\n");
            break;
    })
    DEBUG_PRINT("Stack_Verify completed.\n\n");
    return err;
}

void _StackInit_(Stack_t *stk, ssize_t capacity DEBUG_PAR)
{
    GONDON_STK(stk, "WARNING_FATAL: Struct <Stack_t> isn't created");
    StackElem_t *array = (StackElem_t *) calloc(capacity + 2, sizeof(StackElem_t));
    array[0] = (StackElem_t) STK_CANARY;
    array[capacity + 1] = (StackElem_t) STK_CANARY;
    stk->real_stack = array;
    stk->stack = array + 1;
    stk->capacity = capacity;
    stk->size = 0;

    DEBUG(log_file = fopen(LOGFILE, "w");)
    STACK_OK
}

void _StackPush_(Stack_t *stk, StackElem_t elem DEBUG_PAR)
{
    STACK_OK

    if (stk->size < stk->capacity) {
        stk->stack[stk->size++] = elem;
        return;
    }
    stk->stack = ReSize(stk, UP);

    GONDON_STK(stk->size < stk->capacity, "Size >= capacity after <realloc>. Eto fiasko, bratan")
    stk->stack[stk->size++] = elem;

    STACK_OK
}

StackElem_t *ReSize(Stack_t *stk, int mod)
{
    assert(stk);

    switch(mod) {
        case UP:
            stk->capacity <<= 1;    //De facto *2
            break;
        case DOWN:
            stk->capacity >>= 1;    //De fuckto /2
            break;
    }
    StackElem_t *pre_stack = NULL;
    assert((pre_stack = (StackElem_t *) realloc(stk->real_stack, (stk->capacity + 2) * sizeof(StackElem_t))) != NULL);
    stk->real_stack[stk->capacity + 1] = (StackElem_t) STK_CANARY;

    return pre_stack + 1;
}

StackElem_t _StackPop_(Stack_t *stk DEBUG_PAR)
{
    STACK_OK

    GONDON_STK(stk->size > 0, "WARNING_FATAL: Stack haven't elements for StackPop")
    StackElem_t sin_popa = stk->stack[--stk->size];
    stk->stack[stk->size] = VOID;

    if ((stk->capacity >> 2) > stk->size) {
        stk->stack = ReSize(stk, DOWN);
    }

    STACK_OK
    return sin_popa;
}

void _StackDestroy_(Stack_t *stk DEBUG_PAR)
{
    STACK_OK
    DEBUG_STACK_DUMP(stk)

    for (ssize_t i = 0; i < stk->size; i++)
        stk->stack[i] = VOID;
    free(stk->real_stack);
    *stk = {};

    DEBUG(fclose(log_file);)
}

void StackDump(Stack_t *stk)
{
    DEBUG_PRINT("~~~~~~~~~~~~~~~~~~~~DEBUG~~~~~~~~~~~~~~~~~~~~\n");
    DEBUG_PRINT("LEFT (struct) CANARY  = %lg\n", (StackElem_t) CANARY);
    DEBUG_PRINT("RIGHT (struct) CANARY = %lg\n", (StackElem_t) CANARY);
    DEBUG_PRINT("LEFT STK_CANARY = %lg\n", (StackElem_t) STK_CANARY);
    DEBUG_PRINT("LEFT STK_CANARY = %lg\n\n", (StackElem_t) STK_CANARY);
    if (stk == NULL) {
        DEBUG_PRINT("Pointer on struct <Stack_t> is NULL\n"
                    "Have not object for print\n"
                    "~~~~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~~~~\n");
        return;
    }
    DEBUG_PRINT("Struct Stack_t<%p> {\n", stk->stack);
    DEBUG_PRINT("LEFT CANARY = %lg; %s\n", stk->canary_t1, (stk->canary_t1 == CANARY) ? "" : "<--ERROR")
    DEBUG_PRINT("Stack<%p>; %s\n", stk->real_stack, (stk->real_stack != NULL) ? "" : "<--ERROR")
    DEBUG_PRINT("Stack<%p>; %s\n", stk->stack, (stk->size >= 0) ? "" : "<--ERROR")
    DEBUG_PRINT("capacity = %zu; %s\n", stk->capacity, (stk->capacity >= 0) ? "" : "<--ERROR")
    DEBUG_PRINT("size = %zu; %s\n", stk->size, (stk->size >= 0) ? "" : "<--ERROR")
    DEBUG_PRINT("RIGHT CANARY = %lg; %s\n", stk->canary_t2, (stk->canary_t2 == CANARY) ? "" : "<--ERROR")
    DEBUG_PRINT("}\n")

    if (stk->size <= 0)
        DEBUG_PRINT("STACK HAVE NO MORE 0 ELEMENTS\n");

    DEBUG_PRINT("Print Stack:\n--------------------\n");
    DEBUG_PRINT("*LEFT STK_CANARY* = %lg %s\n", stk->real_stack[0],
                                        (stk->real_stack[0] == STK_CANARY) ? "" : "<--ERROR");
    for (ssize_t i = 0; i < stk->capacity; i++) {
        if (i < stk->size) {
            DEBUG_PRINT(" * [%zd]\t%lg\n", i, stk->stack[i]);
        }
        else {
            DEBUG_PRINT("   [%zd]\t%d\n", i, VOID);
        }
    }
    DEBUG_PRINT("*RIGHT STK_CANARY* = %lg %s\n", stk->real_stack[stk->capacity + 1],
                        (stk->real_stack[stk->capacity + 1] == STK_CANARY) ? "" : "<--ERROR");
    DEBUG_PRINT("--------------------\nEnd of print.\n");
    DEBUG_PRINT("~~~~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~~~~\n");
}

void _StackPrint_(Stack_t *stk DEBUG_PAR)
{
    STACK_OK

    for (ssize_t i = 0; i < stk->size; i++)
        printf("%d  ", stk->stack[i]);
    if (stk->size > 0)
        printf("\n");
}
//TODO: hash