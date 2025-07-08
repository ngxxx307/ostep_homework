#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
    int x = 5;
    int rc = fork();

    if (rc < 0) {
        // Fork failed
        fprintf(stderr, "Fork failed\n");
        return 1;
    } else if (rc == 0) {
        // Child process
        printf("Hello from child process with PID %d. x=%d\n", getpid(), x);
    } else {
        // Parent process
        printf("Hello from parent process with PID %d and child PID %d, x=%d\n", getpid(), rc, x);
    }
    return 0;
}