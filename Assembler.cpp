#include "VM_Libs_Enums.h"

#define MAXFILENAME 20
#define MAXCMDLEN 15
#define MAXLABELSLEN 1000

char file_in_name[MAXFILENAME]  = "in.asm";
char file_out_name[MAXFILENAME] = "code.txt";

#define GONDON(condition)                                                   \
    if (!(condition)) {                                                     \
        fprintf(stderr, "File: %s:%d, Function: %s\nGONDON HELP YOU\n",     \
            __FILE__, __LINE__, __FUNCTION__);                              \
        abort();                                                            \
    }

#define CHECKSIZE   if (buffon->ip >= buffon->size) {               \
                        ERRTEXT("LIST OF COMMANDS IS FULL\n");      \
                        return CMD_LIST_FULL;                       \
                    }
#define ERRTEXT(...) fprintf(stderr, __VA_ARGS__);
#define ADD_COMMAND(name)                           \
    CHECKSIZE                                       \
    buffon->cmd_list[buffon->ip++] = name;

enum ErrCodes {
    NONERR            = 0,
    ABSENCE_ARG       = 1,
    CMD_LIST_FULL     = 2,
    NON_COMMAND       = 3,
    LABELS_LIST_FULL  = 4,
    FATAL_ERR         = 5,
};

enum nFields {
    ZERO = 0,
    ONE  = 1,
    TWO  = 2,
};

enum AssMode {
    FIRST  = 1,
    SECOND = 2,
};

struct Data {
    char *buffer;
    size_t size;

    int *cmd_list;
    size_t ip;

    int *labels_list;
};

struct Trans {
    char *bufpointer_copy;
    char cmdword[MAXCMDLEN] = {};
    int  nFields;
    int  check_scan;
    int  par_value;
    int  err;
};

char *OwnCalloc(size_t nmem, size_t size, int value);

void WriteInBuf(const int file_ind, char *buffer, const size_t file_size);
size_t Fsize(int file_ind);
void ReadFile(Data *buffon);
int TranslateDataer(Data *buffon, int mode);

int Assemblyator(Data *buffon, int mode);
int NumberAssem(Data *buffon, int mode);
void WriteInFile(Data *buffon);

int ReadLABELpar(Data *buffon, Trans *helper);
int ReadREGpar(Data *buffon, Trans *helper);

int AddReg(Data *buffon, char *cmdword);

int main(int argc, char *argv[])
{
    if (argc > 2 && strcmp(argv[1], ">") == 0)
        strncpy(file_out_name, argv[2], strlen(argv[2]));

    Data buffon = {};
    ReadFile(&buffon);

    TranslateDataer(&buffon, FIRST);
    // printf("ERRORCODE 1st ASS: %d\n", err);

    TranslateDataer(&buffon, SECOND);                      // Second assembling
    // printf("ERRORCODE 2nd ASS: %d\n", err);

    WriteInFile(&buffon);
    return 0;
}

int Assemblyator(Data *buffon, int mode)
{
    GONDON(buffon)

    Trans helper = {};
    helper.bufpointer_copy = buffon->buffer;

    return NONERR;
}

int NumberAssem(Data *buffon, int mode)
{
    GONDON(buffon)
    switch (mode) {
        case FIRST:
            buffon->cmd_list = (int *) calloc(buffon->size, sizeof(int));
            GONDON(buffon->cmd_list)
            buffon->labels_list = (int *) OwnCalloc(MAXLABELSLEN, sizeof(int), -1);
            GONDON(buffon->labels_list)
            break;
        case SECOND:
            buffon->ip = 0;
            break;
        default:
            ERRTEXT("HULIGAN-PROGRAMMIST\n");
            return FATAL_ERR;
    }
    return NONERR;
}

int TranslateDataer(Data *buffon, int mode)
{
    GONDON(buffon)
    switch (mode) {
        case FIRST:
            GONDON(buffon->cmd_list = (int *) calloc(buffon->size, sizeof(int)))
            GONDON(buffon->labels_list = (int *) OwnCalloc(MAXLABELSLEN, sizeof(int), -1))
            break;
        case SECOND:
            buffon->ip = 0;
            break;
        default:
            ERRTEXT("HULIGAN-PROGRAMMIST\n");
            return FATAL_ERR;
    }
    // printf("%d\n", buffon->labels_list[0]);
    Trans helper = {};
    helper.bufpointer_copy = buffon->buffer;

    while (sscanf(helper.bufpointer_copy, "%s%n", helper.cmdword, &helper.nFields) > 0) {
        helper.bufpointer_copy += helper.nFields;

        if (helper.cmdword[0] == ';') {
            while (*(helper.bufpointer_copy++) != '\n') {}

            if (*helper.bufpointer_copy == '\0')
                break;
            continue;
        }
        if (helper.cmdword[0] == ':') {
            helper.check_scan = sscanf(helper.cmdword, ":%d%n", &helper.par_value, &helper.nFields);
            if (helper.check_scan <= 0) {
                ERRTEXT("NON PARAMETRES AFTER <%s>\n", helper.cmdword)
                return ABSENCE_ARG;
            }
            if (helper.par_value >= MAXLABELSLEN) {
                ERRTEXT("LABEL <%d> IS TOO LARGE\n", helper.par_value);
                return LABELS_LIST_FULL;
            }
            buffon->labels_list[helper.par_value] = buffon->ip;
            continue;
        }
        if (strcmp(helper.cmdword, "push") == 0) {
            ADD_COMMAND(PUSH)
            helper.check_scan = sscanf(helper.bufpointer_copy, "%d%n", &helper.par_value, &helper.nFields);

            if (helper.check_scan <= 0) {
                ERRTEXT("NON PARAMETRES AFTER <%s>\n", helper.cmdword)
                return ABSENCE_ARG;
            }
            helper.bufpointer_copy += helper.nFields;
            ADD_COMMAND(helper.par_value)
            continue;
        }
        if (strcmp(helper.cmdword, "add") == 0) {
            ADD_COMMAND(ADD)
            continue;
        }
        if (strcmp(helper.cmdword, "sub") == 0) {
            ADD_COMMAND(SUB)
            continue;
        }
        if (strcmp(helper.cmdword, "div") == 0) {
            ADD_COMMAND(DIV)
            continue;
        }
        if (strcmp(helper.cmdword, "mul") == 0) {
            ADD_COMMAND(MUL)
            continue;
        }
        if (strcmp(helper.cmdword, "out") == 0) {
            ADD_COMMAND(OUT)
            continue;
        }
        if (strcmp(helper.cmdword, "hlt") == 0) {
            ADD_COMMAND(HLT)
            continue;
        }
        if (strcmp(helper.cmdword, "pushr") == 0) {
            ADD_COMMAND(PUSHR)
            if ((helper.err = ReadREGpar(buffon, &helper)) != 0)
                return helper.err;
            continue;
        }
        if (strcmp(helper.cmdword, "popr") == 0) {
            ADD_COMMAND(POPR)
            if ((helper.err = ReadREGpar(buffon, &helper)) != 0)
                return helper.err;
            continue;
        }
        if (strcmp(helper.cmdword, "jmp") == 0) {
            ADD_COMMAND(JMP)
            if ((helper.err = ReadLABELpar(buffon, &helper)) != 0)
                return helper.err;
            continue;
        }
        if (strcmp(helper.cmdword, "ja") == 0) {
            ADD_COMMAND(JA)
            if ((helper.err = ReadLABELpar(buffon, &helper)) != 0)
                return helper.err;
            continue;
        }
        if (strcmp(helper.cmdword, "jae") == 0) {
            ADD_COMMAND(JAE)
            if ((helper.err = ReadLABELpar(buffon, &helper)) != 0)
                return helper.err;
            continue;
        }
        if (strcmp(helper.cmdword, "jb") == 0) {
            ADD_COMMAND(JB)
            if ((helper.err = ReadLABELpar(buffon, &helper)) != 0)
                return helper.err;
            continue;
        }
        if (strcmp(helper.cmdword, "jbe") == 0) {
            ADD_COMMAND(JBE)
            if ((helper.err = ReadLABELpar(buffon, &helper)) != 0)
                return helper.err;
            continue;
        }
        if (strcmp(helper.cmdword, "je") == 0) {
            ADD_COMMAND(JE)
            if ((helper.err = ReadLABELpar(buffon, &helper)) != 0)
                return helper.err;
            continue;
        }
        if (strcmp(helper.cmdword, "jne") == 0) {
            ADD_COMMAND(JNE)
            if ((helper.err = ReadLABELpar(buffon, &helper)) != 0)
                return helper.err;
            continue;
        }
        if (strcmp(helper.cmdword, "in") == 0) {
            ADD_COMMAND(IN)
            continue;
        }
        if (strcmp(helper.cmdword, "call") == 0) {
            ADD_COMMAND(CALL)
            if ((helper.err = ReadLABELpar(buffon, &helper)) != 0)
                return helper.err;
            continue;
        }
        if (strcmp(helper.cmdword, "ret") == 0) {
            ADD_COMMAND(RET)
            continue;
        }
        ERRTEXT("COMMAND NAME <%s> IS INCORRECT\n", helper.cmdword)
        return NON_COMMAND;
    }
    return NONERR;
}

int ReadREGpar(Data *buffon, Trans *helper)
{
    GONDON(buffon)
    GONDON(helper)

    helper->check_scan = sscanf(helper->bufpointer_copy, "%s%n", helper->cmdword, &helper->nFields);
    // printf("%s\n", cmdword);
    if (helper->check_scan <= 0) {
        ERRTEXT("NON PARAMETRES AFTER <%s>\n", helper->cmdword)
        return ABSENCE_ARG;
    }

    helper->bufpointer_copy += helper->nFields;

    if (AddReg(buffon, helper->cmdword) != 0) {
        return NON_COMMAND;
    }
    return NONERR;
}

int ReadLABELpar(Data *buffon, Trans *helper)
{
    GONDON(buffon)
    GONDON(helper)

    helper->check_scan = sscanf(helper->bufpointer_copy, " :%d%n", &helper->par_value, &helper->nFields);        // Specifier %*d made for EOF
    //printf("%d\n", par_value);
    //printf("%d\n", check_scan);
    if (helper->check_scan <= 0) {
        ERRTEXT("NON PARAMETRES AFTER <%s>\n", helper->cmdword)
        return ABSENCE_ARG;
    }
    if (helper->par_value >= MAXLABELSLEN) {
        ERRTEXT("LABEL <%d> IS TOO LARGE\n", helper->par_value);
        return LABELS_LIST_FULL;
    }
    helper->bufpointer_copy += helper->nFields;
    ADD_COMMAND(buffon->labels_list[helper->par_value])
    return NONERR;
}

int AddReg(Data *buffon, char *cmdword)
{
    GONDON(buffon)
    GONDON(cmdword)

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

void ReadFile(Data *buffon)
{
    GONDON(buffon)

    int file_in_d = 0;
    GONDON((file_in_d = open(file_in_name, O_RDONLY, 0)) != -1);

    size_t file_size = Fsize(file_in_d);
    GONDON(buffon->buffer = (char *) calloc(file_size + 1, sizeof(char)))
    buffon->buffer[file_size] = '\0';    //For non errors in comments (;)

    WriteInBuf(file_in_d, buffon->buffer, file_size);
    buffon->size = file_size;

    close(file_in_d);
}

void WriteInBuf(const int file_ind, char *buffer, const size_t file_size)
{
    GONDON(buffer);
    GONDON(read(file_ind, buffer, file_size) > 0);
}

size_t Fsize(int file_ind)
{
    struct stat file_info = {};
    fstat(file_ind, &file_info);
    return file_info.st_size;
}

void WriteInFile(Data *buffon)
{
    GONDON(buffon)
    FILE *file_out = NULL;
    GONDON(file_out = fopen(file_out_name, "w"));

    for (size_t i = 0; i < buffon->ip; i++) {
        fprintf(file_out, "%d ", buffon->cmd_list[i]);
    }

    fclose(file_out);
}

char *OwnCalloc(size_t nmem, size_t size, int value)
{
    char *pointer = NULL;
    GONDON(pointer = (char *) malloc(nmem * size))//deystvu=it bolbo
    GONDON(pointer = (char *) memset(pointer, value, nmem * size))

    return pointer;
}