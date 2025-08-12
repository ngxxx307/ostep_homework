#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t head_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct __node
{
    int value;
    struct __node *next;
    pthread_mutex_t lock;
} node;

node *createNode(int value)
{
    node *newNode = (node *)malloc(sizeof(node));
    if (newNode == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newNode->value = value;
    newNode->next = NULL;
    pthread_mutex_init(&newNode->lock, NULL);
    return newNode;
}

void acquire_lock_and_release(node *old)
{
    pthread_mutex_lock(&old->next->lock);
    pthread_mutex_unlock(&old->lock);
}

void insert_at_start(node **head, node *newNode)
{
    pthread_mutex_lock(&head_lock);
    newNode->next = *head;
    *head = newNode;
    pthread_mutex_unlock(&head_lock);
}

void insert_at_end(node **head, node *newNode)
{
    node *currNode = *head;
    while (currNode->next != NULL)
    {
        currNode = currNode->next;
    }
    currNode->next = newNode;
}

void delete_at_start(node **head)
{
    node *temp = *head;
    if (temp->next == NULL)
        return;
    head = &temp->next;

    free(temp);
}

void delete_at_end(node **head, node *newNode)
{
    node *curr = *head;
    while (curr->next != NULL)
    {
        curr = curr->next;
    }
    free(curr);
}

void display_list(node **head)
{
    node *currNode = *head;

    while (currNode != NULL)
    {
        printf("Current Node %p: %d\n", currNode, currNode->value);
        currNode = currNode->next;
    }
}

int main()
{
    node *head = createNode(1);

    display_list(&head);

    return 0;
}