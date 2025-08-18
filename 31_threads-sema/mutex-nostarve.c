#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "common_threads.h"

#include <semaphore.h>
#include <fcntl.h> // For O_CREAT, O_EXCL
//
// Here, you have to write (almost) ALL the code. Oh no!
// How can you show that a thread does not starve
// when attempting to acquire this mutex you build?
//

typedef struct __node
{
    struct __node *next;
    sem_t *s;
} node;

typedef struct __ns_mutex_t
{
    node *head;
    node *tail;
    sem_t *queue_lock;
    int count;
} ns_mutex_t;

ns_mutex_t lock;

node *new_node(int count)
{
    node *newNode = malloc(sizeof(node));
    if (newNode == NULL)
    {
        perror("malloc failed");
        return NULL;
    }
    newNode->next = NULL;

    // Create a buffer to hold the unique semaphore name.
    char sem_name[255];

    // Format the unique name into the buffer. Names must start with '/'.
    // Using snprintf is safer than sprintf.
    snprintf(sem_name, sizeof(sem_name), "/ns_sem_%d", count);

    // Unlink any semaphore with this name from a previous crashed run.
    sem_unlink(sem_name);

    newNode->s = sem_open(sem_name, O_CREAT, 0644, 0);
    if (newNode->s == SEM_FAILED)
    {
        perror("sem_open failed");
        free(newNode);
        return NULL;
    }
    return newNode;
}
void ns_mutex_init(ns_mutex_t *m)
{
    sem_unlink("/mutex");
    sem_unlink("/queue_lock");

    m->queue_lock = sem_open("/queue_lock", O_CREAT, 0644, 1);
    m->head = NULL;
    m->tail = NULL;
    m->count = 0;
}

void ns_mutex_acquire(ns_mutex_t *m)
{
    // printf("acquiring\n");
    // printf("head: %p\n", m->head);
    // printf("true: %d\n", m->head == NULL);
    sem_wait(m->queue_lock);
    m->count++;
    node *newNode = new_node(m->count);
    if (m->head == NULL)
    {
        printf("head null\n");
        m->head = newNode;
        m->tail = newNode;
        sem_post(m->queue_lock);
        return;
    }
    else
    {
        m->tail->next = newNode;
        m->tail = newNode;
        sem_post(m->queue_lock);
        sem_wait(newNode->s);
        return;
    }
}

void ns_mutex_release(ns_mutex_t *m)
{
    sem_wait(m->queue_lock);
    m->head = m->head->next;
    if (m->head != NULL)
    {
        sem_post(m->head->s);
    }
    else
    {
        m->tail = NULL;
    }
    sem_post(m->queue_lock);
}

void *worker(void *arg)
{
    ns_mutex_acquire(&lock);
    int thread_id = (int)pthread_self();

    if (thread_id % 3 == 0)
        sleep(1);

    printf("%d is running...\n", thread_id);
    ns_mutex_release(&lock);
    printf("%d finish running...\n", thread_id);
    return NULL;
}

int main(int argc, char *argv[])
{
    int count = atoi(argv[1]);
    printf("count: %d\n", count);
    pthread_t threads[count];
    ns_mutex_init(&lock);

    printf("parent: begin\n");
    int i;
    for (i = 0; i < count; i++)
    {
        Pthread_create(&threads[i], NULL, worker, NULL);
    }

    for (int i = 0; i < count; i++)
    {
        Pthread_join(threads[i], NULL);
    }
    printf("parent: end\n");
    return 0;
}
