#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "common_threads.h"
#include <semaphore.h>
#include <fcntl.h> // For O_CREAT, O_EXCL

sem_t *s;

void *child(void *arg)
{
    printf("child\n");
    // use semaphore here
    sleep(1);
    sem_post(s);
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t p;
    printf("parent: begin\n");
    // init semaphore here
    s = sem_open("/mysemaphore", O_CREAT, 0644, 0);
    if (s == SEM_FAILED)
    {
        perror("sem_open");
        return 1;
    }
    Pthread_create(&p, NULL, child, NULL);
    // use semaphore here
    sem_wait(s);
    printf("parent: end\n");
    return 0;
}
