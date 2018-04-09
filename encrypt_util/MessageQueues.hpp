#ifndef MessageQueues_hpp
#define MessageQueues_hpp

#include <iostream>
#include <pthread.h>

class node{
public:
    char * str;
    unsigned int str_len;
    void *data;
    unsigned int index;
    node *next;
};

class Queue {
public:
    pthread_mutex_t m;
    unsigned int totalEntries;
    node *front;
    node *back;
    
    Queue () {
        front = NULL;
        back = NULL;
        totalEntries = 0;
        pthread_mutex_init(&m, NULL);
    }
    ~Queue () {
        front = NULL;
        back = NULL;
        pthread_mutex_destroy(&m);
    }
};

void enqueue(Queue *messageQueue, void *data);
node* peak(Queue *messageQueue);
node* dequeue(Queue *messageQueue);

#endif /* MessageQueues_hpp */
