#include "VM_Libs_Enums.h"

#define MAX_FILE_LEN 30
#define MAX_CMD_LEN 15
#define MAX_LABELS_LEN 100

#define NON_LABEL -1

char file_in_name[MAX_FILE_LEN]  = "in.asm";
char file_out_name[MAX_FILE_LEN] = "code.txt";

#define GONDON(condition)                                                   \
    if (!(condition)) {                                                     \
        fprintf(stderr, "File: %s:%d, Function: %s\nGONDON HELP YOU\n",     \
            __FILE__, __LINE__, __FUNCTION__);                              \
        abort();                                                            \
    }
#define ERRTEXT(...) fprintf(stderr, __VA_ARGS__);

#define ADD_COMMAND(name)                           \
    if (buffon->ip >= buffon->size) {               \
        ERRTEXT("LIST OF COMMANDS IS FULL\n")       \
        context->err = CMD_LIST_FULL;               \
        return;                                     \
    }                                               \
    buffon->cmd_list[buffon->ip++] = name;

#define SCAN_CHECK  if (assist.check_scan <= 0) {                                   \
                        ERRTEXT("NON PARAMETRES AFTER <%s>\n", context->cmdword)    \
                        context->err = ABSENCE_ARG;                                 \
                        return;                                                     \
                    }
enum ErrCodes {
    NON_ERR           = 0,
    ABSENCE_ARG       = 1,
    CMD_LIST_FULL     = 2,
    NON_COMMAND       = 3,
    LABELS_LIST_FULL  = 4,
    FATAL_ERR         = 5,
};

enum ComPar {
    NONPAR    = 0,
    PAR_STR   = 1,
    PAR_NUM   = 2,
    PAR_LBL   = 3,
};

enum AssMode {
    FIRST  = 1,
    SECOND = 2,
};

struct Label {
    ssize_t ip = NON_LABEL;
    unsigned long hash;
};

struct Assem {
    char *buffer;
    size_t size;

    ParType *cmd_list;
    size_t ip;

    Label *labels_list;
    size_t ip_lbl = 0;
};

struct Trans {
    char *bufpointer_copy;
    char cmdword[MAX_CMD_LEN] = {};
    bool assem_continue = true;
    int  err;
};

struct Scan {
    int check_scan;
    int nFields;
    ParType par_value;
};

struct Command_t {
    int num;
    const char *name;
    const int par;
    unsigned long hash;
};

char *OwnCalloc(size_t nmem, size_t size, int value);

void WriteInBuf(const int file_ind, char *buffer, const size_t file_size);
size_t Fsize(int file_ind);
void ReadFile(Assem *buffon);

Command_t *CreateComStructs();
int CalcCommandsHash(Command_t *commands);
unsigned long CalcHash_djb(const char *word);
int TranslateAssemer(Assem *buffon, int mode);

int Assemblyator(Assem *buffon, Command_t *commands,  int mode);
void NumberAssem(Assem *buffon, Trans *context, int mode);
void CommandHandler(Assem *buffon, Command_t *commands, Trans *context);

bool SpecSymbols(Assem *buffon, Trans *context);
void ReadLBL(Assem *buffon, Trans *context);
void ReadComment(Trans *context);

void CheckParametres(Assem *buffon, Trans* context, int mode);
void ReadPar_NUM(Assem *buffon, Trans *context);
void ReadPar_STR(Assem *buffon, Trans *context);
void ReadPar_LBL(Assem *buffon, Trans *context);

void AddReg(Assem *buffon, Trans *context);

void WriteInFile(Assem *buffon);

int main(int argc, char *argv[])
{
    if (argc > 1) {
        strncpy(file_in_name, argv[1], strlen(argv[1]));
    }

    Assem buffon = {};
    ReadFile(&buffon);

    Command_t *commands = CreateComStructs();
    CalcCommandsHash(commands);

    //for (int i = 0; i < NUMBER_COMMANDS; i++) {
    //    printf("{%d, \"%s\", %lld},\n", commands[i].num, commands[i].name, commands[i].hash);
    //}
    int err = Assemblyator(&buffon, commands, FIRST);
    printf("ERRORCODE 1st ASS: %d\n", err);
    err = Assemblyator(&buffon, commands, SECOND);
    printf("ERRORCODE 2nd ASS: %d\n", err);

    WriteInFile(&buffon);
    return 0;
}

Command_t *CreateComStructs()
{
    static Command_t commands[NUMBER_COMMANDS] = {
        {PUSH,  "push",  PAR_NUM,   0},
        {ADD,   "add",   NONPAR,    0},
        {SUB,   "sub",   NONPAR,    0},
        {DIV,   "div",   NONPAR,    0},
        {MUL,   "mul",   NONPAR,    0},
        {OUT,   "out",   NONPAR,    0},
        {HLT,   "hlt",   NONPAR,    0},
        {PUSHR, "pushr", PAR_STR,   0},
        {POPR,  "popr",  PAR_STR,   0},
        {JMP,   "jmp",   PAR_LBL,   0},
        {JA,    "ja",    PAR_LBL,   0},
        {JAE,   "jae",   PAR_LBL,   0},
        {JB,    "jb",    PAR_LBL,   0},
        {JBE,   "jbe",   PAR_LBL,   0},
        {JE,    "je",    PAR_LBL,   0},
        {JNE,   "jne",   PAR_LBL,   0},
        {IN,    "in",    NONPAR,    0},
        {CALL,  "call",  PAR_LBL,   0},
        {RET,   "ret",   NONPAR,    0},
        {SQRT,  "sqrt",  NONPAR,    0},
        {PUSHM, "pushm", PAR_NUM,   0},
        {POPM,  "popm",  PAR_NUM,   0},
    };
    return commands;
}

int CalcCommandsHash(Command_t *commands)
{
    GONDON(commands)

    for (size_t i = 0; i < NUMBER_COMMANDS; i++) {
        commands[i].hash = CalcHash_djb(commands[i].name);
    }
    return NON_ERR;
}

unsigned long CalcHash_djb(const char *word)
{
    GONDON(word)
    unsigned long hash = 5381;

    while (*word != '\0') {
        hash = ((hash << 5) + hash) + *(word++);
    }
    return hash;
}

int Assemblyator(Assem *buffon, Command_t *commands, int mode)
{
    GONDON(buffon)

    Trans context = {};     // Жопа транса

    NumberAssem(buffon, &context, mode);
    if (context.err != 0)
        return context.err;

    context.bufpointer_copy = buffon->buffer;

    Scan assist = {};
    while (context.assem_continue && sscanf(context.bufpointer_copy, "%s%n", context.cmdword, &assist.nFields) > 0) {
        context.bufpointer_copy += assist.nFields;
        CommandHandler(buffon, commands, &context);
        if (context.err != 0)
            return context.err;
    }
    return context.err;
}

void NumberAssem(Assem *buffon, Trans *context, int mode)
{
    GONDON(buffon)
    switch (mode) {
        case FIRST:
            buffon->cmd_list = (double *) calloc(buffon->size, sizeof(int));
            GONDON(buffon->cmd_list)
            buffon->labels_list = (Label *) OwnCalloc(MAX_LABELS_LEN, sizeof(Label), NON_LABEL);
            GONDON(buffon->labels_list)
            break;
        case SECOND:
            buffon->ip = 0;
            buffon->ip_lbl = 0;
            break;
        default:
            ERRTEXT("THIS NUMBER OF ASSEMBLING IS UNDEFINED\n");
            context->err = FATAL_ERR;
            break;
    }
    return;
}

void CommandHandler(Assem *buffon, Command_t *commands, Trans *context)
{
    GONDON(buffon)
    GONDON(context)

    if (SpecSymbols(buffon, context))
        return;
    if (context->err != 0)
        return;

    unsigned long com_hash = CalcHash_djb(context->cmdword);

    int i = 0;
    while(i < NUMBER_COMMANDS) {
        if (com_hash != commands[i].hash) {
            i++;
            continue;
        }
        ADD_COMMAND(commands[i].num)
        CheckParametres(buffon, context, commands[i].par);
        break;
    }
    if (i >= NUMBER_COMMANDS) {
        ERRTEXT("COMMAND NAME <%s> IS INCORRECT\n", context->cmdword)
        context->err = NON_COMMAND;
        return;
    }
    return;
}

bool SpecSymbols(Assem *buffon, Trans *context)
{
    GONDON(buffon)
    GONDON(context)

    switch (context->cmdword[0]) {
        case ';':
            ReadComment(context);
            break;
        case ':':
            ReadLBL(buffon, context);
            break;
        default:
            return false;
    }
    return true;
}

void ReadComment(Trans *context)
{
    while (*(context->bufpointer_copy) != '\n' && *(context->bufpointer_copy) != '\0') {
        context->bufpointer_copy++;
    }
    if (*context->bufpointer_copy == '\0') {
        context->assem_continue = false;
        return;
    }
    context->bufpointer_copy++;
}

void ReadLBL(Assem *buffon, Trans *context)
{
    GONDON(buffon)
    GONDON(context)

    Scan assist = {};
    // printf("BEFORE: %s\n", context->cmdword);
    assist.check_scan = sscanf(context->cmdword, ":%s%n", context->cmdword, &assist.nFields);
    SCAN_CHECK
    // printf("AFTER: %s\n", context->cmdword);

    unsigned long hash = CalcHash_djb(context->cmdword);

    if (buffon->ip_lbl >= MAX_LABELS_LEN) {
        ERRTEXT("TOO MUCH LABELS IN CODE\n");
        context->err = LABELS_LIST_FULL;
        return;
    }
    (buffon->labels_list[buffon->ip_lbl].hash == hash) ? : buffon->labels_list[buffon->ip_lbl].hash = hash;
    buffon->labels_list[buffon->ip_lbl++].ip = (ssize_t) buffon->ip;
    // for (size_t i = 0; i < MAX_LABELS_LEN; i++) {
    //     printf("ReadLBL:\nLabel[%zu] {ip = %zd; hash = %lu}\n", i, buffon->labels_list[i].ip, buffon->labels_list[i].hash);
    // }
    // printf("\n");
    // buffon->labels_list[(long long) assist.par_value] = buffon->ip;
}

void CheckParametres(Assem *buffon, Trans* context, int mode)
{
    GONDON(buffon)
    GONDON(context)

    switch(mode) {
        case NONPAR:
            return;
        case PAR_NUM:
            ReadPar_NUM(buffon, context);
            break;
        case PAR_STR:
            ReadPar_STR(buffon, context);
            break;
        case PAR_LBL:
            ReadPar_LBL(buffon, context);
            break;
        default:
            ERRTEXT("COMMAND NEED INCORRECT PARAMETR\n")
            context->err = FATAL_ERR;
            break;
    }
    return;
}

void ReadPar_NUM(Assem *buffon, Trans *context)
{
    GONDON(buffon)
    GONDON(context)

    Scan assist = {};

    assist.check_scan = sscanf(context->bufpointer_copy, "%lg%n", &assist.par_value, &assist.nFields);
    SCAN_CHECK

    context->bufpointer_copy += assist.nFields;
    ADD_COMMAND(assist.par_value)
}

void ReadPar_STR(Assem *buffon, Trans *context)
{
    GONDON(buffon)
    GONDON(context)

    Scan assist = {};
    assist.check_scan = sscanf(context->bufpointer_copy, "%s%n", context->cmdword, &assist.nFields);
    SCAN_CHECK

    context->bufpointer_copy += assist.nFields;
    AddReg(buffon, context);
}

void ReadPar_LBL(Assem *buffon, Trans *context)
{
    GONDON(buffon)
    GONDON(context)

    Scan assist = {};

    assist.check_scan = sscanf(context->bufpointer_copy, " :%s%n", context->cmdword, &assist.nFields);        // Specifier %*d made for EOF
    SCAN_CHECK
    
    unsigned long hash = CalcHash_djb(context->cmdword);
    context->bufpointer_copy += assist.nFields;

    int i = 0;
    for (i = 0; i < MAX_LABELS_LEN; i++) {
        if (hash != buffon->labels_list[i].hash)
            continue;
        ADD_COMMAND(buffon->labels_list[i].ip)
        break;
    }
    if (i >= MAX_LABELS_LEN) {
        ADD_COMMAND(NON_LABEL)
    }
    // printf("Cmd_list: \n");
    // for (size_t i = 0; i < buffon->ip; i++) {
    //     printf("%lg ", buffon->cmd_list[i]);
    // }
    // printf("\n\n");
}

void AddReg(Assem *buffon, Trans *context)
{
    GONDON(buffon)
    GONDON(context->cmdword)

    if (strcmp(context->cmdword, "ax") == 0) {
        ADD_COMMAND(AX)
        return;
    }
    if (strcmp(context->cmdword, "bx") == 0) {
        ADD_COMMAND(BX)
        return;
    }
    if (strcmp(context->cmdword, "cx") == 0) {
        ADD_COMMAND(CX)
        return;
    }
    if (strcmp(context->cmdword, "dx") == 0) {
        ADD_COMMAND(DX)
        return;
    }
    ERRTEXT("NO VALID NAME OF REGISTER <%s>\n", context->cmdword);
    context->err = NON_COMMAND;
}

void ReadFile(Assem *buffon)
{
    GONDON(buffon)

    int file_in_d = 0;
    file_in_d = open(file_in_name, O_CREAT, 0644);
    GONDON(file_in_d != -1)

    size_t file_size = Fsize(file_in_d);
    buffon->buffer = (char *) calloc(file_size + 1, sizeof(char));
    GONDON(buffon->buffer)
    buffon->buffer[file_size] = '\0';    //For non errors in comments (;)

    WriteInBuf(file_in_d, buffon->buffer, file_size);
    buffon->size = file_size;

    close(file_in_d);
}

void WriteInBuf(const int file_ind, char *buffer, const size_t file_size)
{
    GONDON(buffer);
    int fields = read(file_ind, buffer, file_size);
    GONDON(fields >= 0)
}

size_t Fsize(int file_ind)
{
    struct stat file_info = {};
    fstat(file_ind, &file_info);
    return (size_t) file_info.st_size;
}

void WriteInFile(Assem *buffon)
{
    GONDON(buffon)
    FILE *file_out = NULL;
    file_out = fopen(file_out_name, "w");
    GONDON(file_out)

    for (size_t i = 0; i < buffon->ip; i++) {
        fprintf(file_out, "%lg ", buffon->cmd_list[i]);
    }

    fclose(file_out);
}

char *OwnCalloc(size_t nmem, size_t size, int value)
{
    char *pointer = NULL;
    pointer = (char *) malloc(nmem * size);
    GONDON(pointer)
    pointer = (char *) memset(pointer, value, nmem * size);
    GONDON(pointer)

    return pointer;
}