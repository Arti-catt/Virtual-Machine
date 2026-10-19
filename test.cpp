#include <stdio.h>
#include <string.h>

int main()
{
    FILE* file1 = fopen("test.txt", "r");
    int number = 0;
    int fields = 0;
    fscanf(file1, "%d%n", &number, &fields);
    printf("%d, fields = %d\n", number, fields);

    fclose(file1);
    return 0;
}