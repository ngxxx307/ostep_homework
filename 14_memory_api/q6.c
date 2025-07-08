#include <stdlib.h>
#include <stdio.h>

int main()
{
    int* data = malloc(100 * sizeof(int));

    free(data);

    printf("%d\n", data[50]);

    return 0;
}