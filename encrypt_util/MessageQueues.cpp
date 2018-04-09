#include "MessageQueues.hpp"

void enqueue(Queue *messageQueue, void *data) {
    node *new_Node = new node();

    if(new_Node == NULL) {
        std::cerr << "memory allocation failed" << std::endl;
        return;
    }
    messageQueue->totalEntries++;
    pthread_mutex_lock(&messageQueue->m);
    new_Node->data = data;
    new_Node->next = NULL;
    
    if( messageQueue->back != NULL)
        messageQueue->back->next = new_Node;
    messageQueue->back = new_Node;
    if(messageQueue->front == NULL)
        messageQueue->front = new_Node;
    pthread_mutex_unlock(&messageQueue->m);
}

node* peak(Queue *messageQueue) {
    
    return messageQueue->front;
}

node* dequeue(Queue *messageQueue) {
    
    node *updateNode;
    if(messageQueue->front == NULL)
        return NULL;
    
    pthread_mutex_lock(&messageQueue->m);
    messageQueue->totalEntries--;
    updateNode = messageQueue->front;
    messageQueue->front = messageQueue->front->next;
    
    if(messageQueue->front == NULL)
        messageQueue->back = NULL;
    
    pthread_mutex_unlock(&messageQueue->m);
    return updateNode;
}
