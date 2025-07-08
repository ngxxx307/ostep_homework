#include <stdio.h>
#include <unistd.h>     // For read(), write(), open(), close(), unlink()
#include <fcntl.h>      // For O_RDWR, O_CREAT
#include <errno.h>      // For errno
#include <string.h> 
#include <sys/time.h>

int main() {
    struct timeval t_begin, t_end;
    char buffer [100];

    int times = 10000000;

    int fd = open("open.c", O_RDONLY ,0644);

    if (fd == -1){
        printf("Error opening file\n");
        return 1;
    }

    gettimeofday(&t_begin, NULL);

    for (int i = 0; i < times; i++) {
        read(fd ,buffer, 0);
    }

    gettimeofday(&t_end, NULL);

    double time_spend = (t_end.tv_sec - t_begin.tv_sec) + (t_end.tv_usec - t_begin.tv_usec) / 1000000.0;
    printf("Time spent: %f\n", time_spend);
     printf("Average time per call: %f microseconds\n", (time_spend * 1000000.0) / times);

    close(fd);

    return 0;
}