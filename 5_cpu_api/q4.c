#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 

int main(){
    int rc = fork();
    if (rc < 0) {
        perror("fork");
        return 1;
    }
    if (rc == 0){
        // 1. execvp
        // char* arg[3];
        // arg[0] = strdup("wc");
        // arg[1] = strdup("q3.c");
        // arg[2] = NULL;
        // execvp(arg[0], arg);

        // 2. execl
        // if (execl("q3", "q3", NULL) == -1){
        //     perror("execl failed\n");
        //     return 1;
        // };

        // 3. execlp
        // if (execlp("wc", "wc", "q3.c", NULL) == -1){
        //     perror("execlp failed\n");
        //     return 1;
        // }
        return 0;
    }
    return 0;
}