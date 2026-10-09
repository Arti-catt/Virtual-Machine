#include <stdio.h>
#include <string.h>

#include "Stack.h"

#define GONDON assert

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
}