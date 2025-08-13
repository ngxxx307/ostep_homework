#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
    int rc1 = fork();
    if (rc1 < 0) {
        // Fork failed
        fprintf(stderr, "Fork failed\n");
        return 1;
    } else if (rc1 == 0) {
        sleep(3);
        printf("Hello from child process 1\n");
        return 0;
    } else {

        int rc2 = fork();
        if (rc2 == 0){
            printf("Hello from child process 2\n");
            return 0;
        }
        // Parent process
        int *status;
        int p = waitpid(rc2, status, 0);
        printf("Hello from parent process with PID %d and child PID %d, wait returns=%d\n", getpid(), rc2, p);
    }
    return 0;
}