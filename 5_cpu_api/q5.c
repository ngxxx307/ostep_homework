#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
    int rc = fork();

    if (rc < 0) {
        // Fork failed
        fprintf(stderr, "Fork failed\n");
        return 1;
    } else if (rc == 0) {
        // Child process
        int p = wait(NULL);
        printf("Hello from child process with PID %d. p=%d\n", getpid(), p);
    } else {
        // Parent process
        int p = wait(NULL);
        printf("Hello from parent process with PID %d and child PID %d, wait returns=%d\n", getpid(), rc, p);
    }
    return 0;
}