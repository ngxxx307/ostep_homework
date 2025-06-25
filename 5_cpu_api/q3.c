#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    if (pipe(fd) == -1){
        perror("pipe");
        return 1;
    }

    int rc = fork();
    
    if (rc < 0){
        fprintf(stderr, "fork failed\n");
        return 1;
    }
    if (rc == 0){
        close(fd[0]); // close the input side
        printf("Child: Hello world!\n");
        fflush(stdout);
        write(fd[1], " ", 1);
        close(fd[1]);
    } else {
        close(fd[1]); // close the output side
        char x;
        read(fd[0], &x, 1);
        printf("Parent: here!\n");
        close(fd[0]);
    }


    return 0;
}