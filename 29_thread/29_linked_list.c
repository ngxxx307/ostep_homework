#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Lock for *head pointer
pthread_mutex_t head_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct __node
{
    int value;
    struct __node *next;
    pthread_mutex_t lock; // lock for accessing and mutating node
} node;

typedef struct __linked_list
{
    node *head;
    pthread_mutex_t lock;
} linked_list;

linked_list *init_linked_list(node *head)
{
    linked_list *list = malloc(sizeof(linked_list));
    pthread_mutex_init(&list->lock, NULL);
    list->head = head;
    return list;
}

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

void insert_at_start(linked_list *list, node *newNode)
{
    pthread_mutex_lock(&list->lock);
    newNode->next = list->head;
    list->head = newNode;
    pthread_mutex_unlock(&list->lock);
}

void insert_at_end(linked_list *list, node *newNode)
{
    pthread_mutex_lock(&list->lock);
    node *currNode = list->head;

    if (currNode == NULL) // Case 1: list is empty
    {
        list->head = newNode;
        pthread_mutex_unlock(&list->lock);
        return;
    }

    // Case 2: List is not empty
    pthread_mutex_lock(&currNode->lock);
    pthread_mutex_unlock(&list->lock);
    node *temp;

    while (currNode->next != NULL)
    {
        temp = currNode->next;
        pthread_mutex_lock(&temp->lock);
        pthread_mutex_unlock(&currNode->lock);
        currNode = temp;
    }
    currNode->next = newNode;
    pthread_mutex_unlock(&currNode->lock);
}

void delete_at_start(linked_list *list)
{
    pthread_mutex_lock(&list->lock);
    node *head = &list->head;
    if (head == NULL)
    {
        pthread_mutex_unlock(&list->lock);
        return;
    }

    head = &head->next;

    free(head);
}

void delete_at_end(linked_list *list, node *newNode)
{
    node *curr = list->head;
    while (curr->next != NULL)
    {
        curr = curr->next;
    }
    free(curr);
}
void display_list(linked_list *list)
{
    node *currNode = list->head;

    while (currNode != NULL)
        while (currNode != NULL)
        {
            printf("Current Node %p: %d\n", currNode, currNode->value);
            currNode = currNode->next;
        }
}

int main()
{
    node *head = createNode(1);
    linked_list *list = init_linked_list(head);

    display_list(list);

    return 0;
}