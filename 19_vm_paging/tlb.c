#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

const int PAGE_SIZE = 1024 * 16;

int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        printf("example usage: ./tlb <no. page> <no. trial>\\n");
        exit(1);
    }

    int jump = PAGE_SIZE / sizeof(int);
    int number_of_page = atoi(argv[1]);
    int number_of_trials = atoi(argv[2]);

    // 1. Allocate array on the heap to avoid stack overflow
    int *a = malloc(jump * number_of_page * sizeof(int));
    if (a == NULL)
    {
        perror("malloc failed");
        exit(1);
    }

    // Initialize array to avoid page faults during timing
    for (int i = 0; i < jump * number_of_page; i++)
    {
        a[i] = 0;
    }

    struct timeval start;
    struct timeval end;
    gettimeofday(&start, 0);

    for (int trial = 0; trial < number_of_trials; trial++)
    {
        for (int i = 0; i < number_of_page * jump; i += jump)
        {
            a[i] += 1;
        }
    }
    gettimeofday(&end, 0);

    // 2. Correctly calculate total time in nanoseconds
    double total_time_ns = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_usec - start.tv_usec) * 1e3;

    // We access number_of_page elements per trial
    printf("Average time per access: %f ns\\n", total_time_ns / (number_of_page * number_of_trials));

    free(a); // 3. Free the allocated memory
    return 0;
}