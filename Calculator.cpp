#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Stack.h"

#define GONDON assert
#define MAXLEN 1000

enum OperMode {
    ADD  =  0,
    SUB  =  1,
    DIV  =  2,
    MUL  =  3,      //Умножение
};

void BaseOperation(Stack_t *stk, int mode);

int main()
{
    Stack_t stk = {};
    StackInit(&stk, 100);
    char command[MAXLEN] = {};
    int number = 0;
    while (scanf("%s", command) != 0) {
        if (strcmp(command, "add") == 0) {
            BaseOperation(&stk, ADD);
        }
        else if (strcmp(command, "sub") == 0) {
            BaseOperation(&stk, SUB);
        }
        else if (strcmp(command, "div") == 0) {
            BaseOperation(&stk, DIV);
        }
        else if (strcmp(command, "mul") == 0) {
            BaseOperation(&stk, MUL);
        }
        else if (strcmp(command, "push") == 0) {
            scanf("%d", &number);
            StackPush(&stk, number);
        }
        else if (strcmp(command, "out") == 0) {
            StackPrint(&stk);
        }
        else if (strcmp(command, "hlt") == 0)
            return 0;
    }
    return 0;
}

void BaseOperation(Stack_t *stk, int mode)
{
    GONDON(stk);
    GONDON(stk->size >= 2);

    StackElem_t second = StackPop(stk);
    StackElem_t result = 0;

    switch (mode) {
        case ADD:
            result = StackPop(stk) + second;
            break;
        case SUB:
            result = StackPop(stk) - second;
            break;
        case DIV:
            GONDON(second != 0);
            result = StackPop(stk) / second;
            break;
        case MUL:
            result = StackPop(stk) * second;
            break;
    }
    StackPush(stk, result);
}