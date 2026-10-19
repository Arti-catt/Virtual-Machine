#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/file.h>
#include <unistd.h>
#include <malloc.h>

#define MAXFILENAME 20
#define MAXCMDLEN 15
#define MAXLABELSLEN 1000

char file_in_name[MAXFILENAME] =  "in.asm";
char file_out_name[MAXFILENAME] = "code.txt";

#define GONDON(condition, text)     \
    if (!(condition)) {     \
        fprintf(stderr, "File: %s:%d, Function: %s\n%s\n",   \
            __FILE__, __LINE__, __FUNCTION__, text);   \
        abort();    \
    }

#define CHECKSIZE   if (buffon->ip >= buffon->size) {     \
                        ERRTEXT("LIST OF COMMANDS IS FULL\n");      \
                        return CMD_LIST_FULL;       \
                    }
#define ADD_COMMAND(name)    CHECKSIZE       \
                            buffon->cmd_list[buffon->ip++] = name;
#define ERRTEXT(...) fprintf(stderr, __VA_ARGS__);

enum ErrCodes {
    NONERR            =   0,
    ABSENCE_ARG       =   1,
    CMD_LIST_FULL     =   2,
    NON_COMMAND       =   3,
    LABELS_LIST_FULL  =   4,
    FATAL_ERR         =   5,
};

enum nFields {
    ZERO = 0,
    ONE  = 1,
    TWO  = 2,
};

enum AssMode {
    FIRST  =  1,
    SECOND =  2,
};

enum Commands {
    PUSH    =   1,
    ADD     =   2,
    SUB     =   3,
    DIV     =   4,
    MUL     =   5,
    OUT     =   6,
    HLT     =   7,
    PUSHR   =   8,
    POP     =   9,
    JMP    =  10,
};

enum Registers {
    AX = 1,
    BX = 2,
    CX = 3,
    DX = 4,
};

struct Buff {
    char *buffer;
    size_t size;

    int *cmd_list;
    size_t ip;

    int *labels_list;
};

char *OwnCalloc(size_t nmem, size_t size, int value);

void WriteInBuf(const int file_ind, char *buffer, const size_t file_size);
size_t Fsize(int file_ind);
void ReadFile(Buff *buffon);
int TranslateBuffer(Buff *buffon, int mode);
void WriteInFile(Buff *buffon);

int AddReg(Buff *buffon, char *cmdword);

int main(int argc, char *argv[])
{
    if (argc > 2 && strcmp(argv[1], ">") == 0)
        strncpy(file_out_name, argv[2], strlen(argv[2]));

    Buff buffon = {};
    ReadFile(&buffon);

    TranslateBuffer(&buffon, FIRST);
    // printf("ERRORCODE 1st ASS: %d\n", err);

    TranslateBuffer(&buffon, SECOND);                      // Second assembling
    // printf("ERRORCODE 2nd ASS: %d\n", err);

    WriteInFile(&buffon);
    return 0;
}

int TranslateBuffer(Buff *buffon, int mode)
{
    GONDON(buffon, "")
    switch (mode) {
        case FIRST:
            GONDON(buffon->cmd_list = (int *) calloc(buffon->size, sizeof(int)), "")
            GONDON(buffon->labels_list = (int *) OwnCalloc(MAXLABELSLEN, sizeof(int), -1), "")
            break;
        case SECOND:
            buffon->ip = 0;
            break;
        default:
            ERRTEXT("HULIGAN-PROGRAMMIST\n");
            return FATAL_ERR;
    }
    // printf("%d\n", buffon->labels_list[0]);
    char *bufpointer_copy = buffon->buffer;             //True buffer pointer is CONST
    char cmdword[MAXCMDLEN] = {};
    int nFields = 0;
    int check_scan = 0;
    int par_value = 0;
    while (sscanf(bufpointer_copy, "%s%n", cmdword, &nFields) > 0) {
        bufpointer_copy += nFields;
        if (cmdword[0] == ';') {
            while (*(bufpointer_copy++) != '\n') {}
            continue;
        }
        if (cmdword[0] == ':') {
            check_scan = sscanf(cmdword, ":%d%n", &par_value, &nFields);
            if (check_scan <= 0) {
                ERRTEXT("NON PARAMETRES AFTER <%s>\n", cmdword)
                return ABSENCE_ARG;
            }
            if (par_value >= MAXLABELSLEN) {
                ERRTEXT("LABEL <%d> IS TOO LARGE\n", par_value);
                return LABELS_LIST_FULL;
            }
            buffon->labels_list[par_value] = buffon->ip;
            continue;
        }
        if (strcmp(cmdword, "push") == 0) {
            ADD_COMMAND(PUSH)
            check_scan = sscanf(bufpointer_copy, "%d%n", &par_value, &nFields);

            if (check_scan <= 0) {
                ERRTEXT("NON PARAMETRES AFTER <%s>\n", cmdword)
                return ABSENCE_ARG;
            }
            bufpointer_copy += nFields;
            ADD_COMMAND(par_value)
            continue;
        }
        if (strcmp(cmdword, "add") == 0) {
            ADD_COMMAND(ADD)
            continue;
        }
        if (strcmp(cmdword, "sub") == 0) {
            ADD_COMMAND(SUB)
            continue;
        }
        if (strcmp(cmdword, "div") == 0) {
            ADD_COMMAND(DIV)
            continue;
        }
        if (strcmp(cmdword, "mul") == 0) {
            ADD_COMMAND(MUL)
            continue;
        }
        if (strcmp(cmdword, "out") == 0) {
            ADD_COMMAND(OUT)
            continue;
        }
        if (strcmp(cmdword, "hlt") == 0) {
            ADD_COMMAND(HLT)
            continue;
        }
        if (strcmp(cmdword, "pushr") == 0 || strcmp(cmdword, "pop") == 0) {
            if (strcmp(cmdword, "pushr") == 0) {
                ADD_COMMAND(PUSHR)
            }
            else {
                ADD_COMMAND(POP)
            }
            check_scan = sscanf(bufpointer_copy, "%s%n", cmdword, &nFields);
            if (check_scan <= 0) {
                ERRTEXT("NON PARAMETRES AFTER <%s>\n", cmdword)
                return ABSENCE_ARG;
            }
            bufpointer_copy += nFields;

            if (AddReg(buffon, cmdword) != 0) {
                return NON_COMMAND;
            }
            continue;
        }
        if (strcmp(cmdword, "jmp") == 0) {
            ADD_COMMAND(JMP)
            check_scan = sscanf(bufpointer_copy, " :%d%*d%n", &par_value, &nFields);        // Specifier %*d made for EOF
            //printf("%d\n", par_value);
            //printf("%d\n", check_scan);
            if (check_scan <= 0) {
                ERRTEXT("NON PARAMETRES AFTER <%s>\n", cmdword)
                return ABSENCE_ARG;
            }
            if (par_value >= MAXLABELSLEN) {
                ERRTEXT("LABEL <%d> IS TOO LARGE\n", par_value);
                return LABELS_LIST_FULL;
            }
            bufpointer_copy += nFields;
            ADD_COMMAND(buffon->labels_list[par_value])
            continue;
        }
        ERRTEXT("COMMAND NAME <%s> IS INCORRECT\n", cmdword)
        return NON_COMMAND;
    }
    return NONERR;
}

int AddReg(Buff *buffon, char *cmdword)
{
    GONDON(buffon, "")
    GONDON(cmdword, "")

    if (strcmp(cmdword, "ax") == 0) {
            ADD_COMMAND(AX)
            return NONERR;
        }
    if (strcmp(cmdword, "bx") == 0) {
            ADD_COMMAND(BX)
            return NONERR;
        }
    if (strcmp(cmdword, "cx") == 0) {
            ADD_COMMAND(CX)
            return NONERR;
        }
    if (strcmp(cmdword, "dx") == 0) {
            ADD_COMMAND(DX)
            return NONERR;
        }
    ERRTEXT("NO VALID NAME OF REGISTER <%s>\n", cmdword);
    return NON_COMMAND;
}

void ReadFile(Buff *buffon)
{
    GONDON(buffon, "")

    int file_in_d = 0;
    GONDON((file_in_d = open(file_in_name, O_RDONLY, 0)) != -1, "");

    size_t file_size = Fsize(file_in_d);
    GONDON(buffon->buffer = (char *) calloc(file_size + 1, sizeof(char)), "")
    buffon->buffer[file_size] = '\n';    //For non errors in comments (;)

    WriteInBuf(file_in_d, buffon->buffer, file_size);
    buffon->size = file_size;

    close(file_in_d);
}

void WriteInBuf(const int file_ind, char *buffer, const size_t file_size)
{
    GONDON(buffer, "");
    GONDON(read(file_ind, buffer, file_size) > 0, "");
}

size_t Fsize(int file_ind)
{
    struct stat file_info = {};
    fstat(file_ind, &file_info);
    return file_info.st_size;
}

void WriteInFile(Buff *buffon)
{
    GONDON(buffon, "")
    FILE *file_out = NULL;
    GONDON(file_out = fopen(file_out_name, "w"), "");

    for (size_t i = 0; i < buffon->ip; i++) {
        fprintf(file_out, "%d ", buffon->cmd_list[i]);
    }

    fclose(file_out);
}

char *OwnCalloc(size_t nmem, size_t size, int value)
{
    char *pointer = NULL;
    GONDON(pointer = (char *) malloc(nmem * size), "")
    GONDON(pointer = (char *) memset(pointer, value, nmem * size), "")

    return pointer;
}