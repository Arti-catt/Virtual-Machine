#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <sys/file.h>

#define FILENAME_IN "Onegin.txt"
#define FILENAME_OUT "Onegin_out.txt"

typedef char byte;

#define GONDON assert

enum Mods {
    LASTCHANGE = 0,
    ORIGINAL   = 1,
};

struct Stringi {
    char *str;
    size_t str_size;
};

struct GlobalData {
    char *text;
    size_t text_size;
    Stringi *index;
    size_t index_size;
};

void ReadAllStrings(GlobalData *file_data);
size_t FileSize(const char *filename);
void FillIndex(Stringi index[], char buffer[], size_t index_size, const int endsymb);
size_t ChangeEndOfStrings(GlobalData *file_data, const int endsymb, const int tmpl);
void PrintText(GlobalData *file_data, FILE *file_outd, Mods mode);
void FreeDinMemory(GlobalData *file_data);

int CompareUp(const void *a, const void *b);
int CompareEnd(const void *a, const void *b);

void qSortir(void *prot_massive, size_t size, size_t size_elem,
                int (*Compare)(const void *a, const void *b));
void Swapchik(size_t size_elem, void *first, void *second);

int Strcmp(const char *str1, const char *str2);

int main()
{
    GlobalData file_data = {};
    ReadAllStrings(&file_data);
    //printf("HERE\n");
    FILE *file_out = fopen(FILENAME_OUT, "w");
    GONDON(file_out != NULL);
    //PrintText(file_data, file_out);
    //fprintf(file_out, "\n");
    //for (int i = 0; i < file_data.index_size; i++)
    //    printf("%p\n", file_data.index[i]);
    //printf("\n");
    qsort(file_data.index, file_data.index_size,
                    sizeof(file_data.index[0]), CompareUp);
    PrintText(&file_data, file_out, LASTCHANGE);
    fprintf(file_out, "\n\n\n");
    //for (int i = 0; i < file_data.index_size; i++)
    //    printf("%p\n", file_data.index[i]);
    qSortir(file_data.index, file_data.index_size,
                    sizeof(file_data.index[0]), CompareEnd);
    PrintText(&file_data, file_out, LASTCHANGE);
    fprintf(file_out, "\n\n\n");
    //printf("tuta\n");
    //ChangeEndOfStrings(&file_data, '\n');
    PrintText(&file_data, file_out, ORIGINAL);
    //printf("i tuta\n");
    //printf("%s\n", file_data.text);
    //for (size_t i = 0; i < file_data.index_size; i++) {
    //    printf("%zu\n", i);
    //    printf("%s\n", file_data.index[i]);
    //}
    //printf("%p -> %c\n", file_data.text, file_data.text[0]);
    //printf("%p -> %c\n", file_data.text, file_data.text[0]);
    FreeDinMemory(&file_data);
    fclose(file_out);
    return 0;
}

void ReadAllStrings(GlobalData *file_data)
{
    GONDON(file_data != NULL);
    int file_ind = 0;
    GONDON((file_ind = open(FILENAME_IN, O_RDONLY, 0)) != -1);

    size_t file_size = FileSize(FILENAME_IN);

    char *pre_buffer = NULL;
    GONDON((pre_buffer = (char *) calloc(file_size + 3, sizeof(char))) != NULL);
    pre_buffer[0] = '\0';               //FICTION ELEMENT FOR SECOND SORT
    pre_buffer[file_size + 1] = '\n';   //END OF LAST STRING
    pre_buffer[file_size + 2] = '\0';   //END OF BUFFER
    char *buffer = &pre_buffer[1];
    ////setvbuf(file_in, buffer, _IONBF, sizeof(char));
    GONDON(read(file_ind, buffer, file_size) > 0);
    file_data->text = buffer;
    file_data->text_size = file_size + 2;
    //printf("%s\n", buffer);
    size_t size_index = ChangeEndOfStrings(file_data, '\n', '\0');
    Stringi *index = {};
    GONDON((index = (Stringi *) calloc(size_index, sizeof(Stringi))) != NULL);
    FillIndex(index, buffer, size_index, '\0');

    file_data->index = index;
    file_data->index_size = size_index;
    close(file_ind);

    //for (size_t i = 0; i < size_index; i++) {
    //    printf("<%s>, len = %zu\n", file_data->index[i].str, file_data->index[i].str_size);
    //}
    //printf("%zu\n", size_index);
    //for (size_t i = 0; i < size_index; i++) {
    //    printf("%p -> %p\n", index[i], index_copy[i]);
    //}
    //printf("HERE2\n");
    //printf("%s\n", buffer);
    //for (int i = 0; i < MAXLINE && index[i] != NULL; i++) {
    //    printf("%d\n", i);
    //    printf("%s\n", index[i]);
    //}
}

void FreeDinMemory(GlobalData *file_data)
{
    free(file_data->text - 1);
    free(file_data->index);
    file_data->text_size = 0;
    file_data->index_size = 0;
}

size_t FileSize(const char *filename)
{
    struct stat file_info = {};
    stat(filename, &file_info);
    return file_info.st_size;
}

void FillIndex(Stringi *index, char *pointer, size_t index_size, const int endsymb)
{
    GONDON(index  != NULL);
    GONDON(pointer != NULL);
    index[0].str = pointer;

    size_t i = 0;
    char * str_size = pointer;
    while (i < index_size && (pointer = strchr(pointer, endsymb)) != NULL
                        && *(pointer + 1) != '\0') {
        index[i].str_size = pointer - str_size;
        str_size = ++pointer;
        index[++i].str = pointer;
    }
    GONDON(i < index_size);
    index[i].str_size = pointer - str_size;
}

size_t ChangeEndOfStrings(GlobalData *file_data, const int endsymb, const int tmpl)
{
    GONDON(file_data != NULL);
    char *searched = file_data->text;
    size_t size_index = 0;
    
    while ((searched = strchr(searched, endsymb)) != NULL
                        && *(searched + 1) != '\0') {
        //printf("%c\n", *searched);
        *(searched++) = tmpl;
        size_index++;
        //printf("%d -> %zu\n", *(searched - 1), ++count);
    }
    *searched = tmpl;
    //printf("%d -> %zu\n", *(searched), ++count);
    //printf("%zu\n", file_data->index_size);
    return size_index + 1;
}

void PrintText(GlobalData *file_data, FILE *file_out, Mods mode)
{
    GONDON(file_data != NULL);

    switch (mode) {
        case ORIGINAL:
            ChangeEndOfStrings(file_data, '\0', '\n');
            //printf("HERE\n");
            //fprintf(file_out, "%s", file_data->text);
            fputs(file_data->text, file_out);
            break;
        case LASTCHANGE:
            for (size_t i = 0; i < file_data->index_size; i++) {
                fputs(file_data->index[i].str, file_out);
                fprintf(file_out, "\n");
            }
            break;
        default:
            GONDON(false);
    }
}

int CompareUp(const void *a, const void *b)
{
    GONDON(a != NULL);
    GONDON(b != NULL);
    Stringi str_a = *(Stringi *) a;
    Stringi str_b = *(Stringi *) b;
    
    return Strcmp(str_a.str, str_b.str);
}

int CompareEnd(const void *a, const void *b)
{
    GONDON(a != NULL);
    GONDON(b != NULL);

    Stringi str_a = *(Stringi *) a;
    Stringi str_b = *(Stringi *) b;

    size_t a_index = str_a.str_size;    //TODO: Structures
    size_t b_index = str_b.str_size;

    while (!isalpha(str_a.str[--a_index])) {}
    while (!isalpha(str_b.str[--b_index])) {}

    while (tolower(str_a.str[a_index]) == tolower(str_b.str[b_index])) {
        if (str_a.str[a_index] == '\0')
            return 0;

        a_index--; b_index--;
    }
    return tolower(str_a.str[a_index]) - tolower(str_b.str[b_index]);
}


void qSortir(void *prot_massive, size_t size, size_t size_elem,
                int (*Compare)(const void *a, const void *b))
{
    GONDON(prot_massive != NULL);
    ssize_t index = 0, right = size - 1;

    if (right <= index)
        return;

    ssize_t last = index;
    byte *massive = (byte *) prot_massive;
    byte *medium = massive;

    for (index = 1; index <= right; index++) {
        if ((*Compare)(massive + index * size_elem, medium) < 0) {
            Swapchik(size_elem, massive + (++last) * size_elem,
                                    massive + index * size_elem);
        }
    }
    Swapchik(size_elem, medium, massive + last * size_elem);

    qSortir(massive, last, size_elem, Compare);
    qSortir(massive + (last + 1) * size_elem, size - last - 1, size_elem, 
            Compare);
}

void Swapchik(size_t size_elem, void *first, void *second)
{
    GONDON(first != NULL);
    GONDON(second != NULL);

    byte *first_nonv = (byte *) first;
    byte *second_nonv = (byte *) second;

    byte temp = 0;
    for (size_t i = 0; i < size_elem; i++) {
        temp = *second_nonv;
        *(second_nonv++) = *first_nonv;
        *(first_nonv++) = temp;
    }
}

int Strcmp(const char *str1, const char *str2)
{
    GONDON(str1 != NULL);
    GONDON(str2 != NULL);

    while (!isalpha(*(str1)))
        str1++;
    while (!isalpha(*(str2))) 
        str2++;

    while (tolower(*(str1)) == tolower(*(str2))) {
        if (*str1 == '\0')
            return 0;

        str1++; str2++;
    }
    return tolower(*str1) - tolower(*str2);
}
