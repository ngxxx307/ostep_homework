#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argv[1] == NULL)
    {
        perror("please specify memory");
        exit(1);
    }
    int memory_mb = atoi(argv[1]);
    printf("argv[1]: %d\n", memory_mb);

    int num_element = (long)memory_mb * 1024 * 1024 / sizeof(int *);
    int **array = malloc(num_element * sizeof(int *));

    int i = 0;
    while (1)
    {
        array[i % num_element] = &i;
        i++;
        if (i < 0)
        {
            i = 0;
        }
    }

    return 0;
}