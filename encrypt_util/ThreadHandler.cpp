#include "ThreadHandler.hpp"

unsigned int actualThreadNumber;
int encryptID;
Queue *writerQueue;
ThreadInfo threadInfolist[50];
ThreadInfo writeThreadInfo;

void XOROperation(unsigned char *buffer, unsigned long bufferLength, unsigned char *key, unsigned long keyLength) {
    int i = 0,j = 0;
    
    while(i < keyLength && j < bufferLength) {
        buffer[j++] ^= key[i++];
    }
}

void *WorkThreadWork(void *info) {
    ThreadInfo *currentInfo = (ThreadInfo*)info;
    ThreadMessage encryptMessage;
    ThreadMessage responseMessage;
    long status;
    responseMessage.id = currentInfo->type;
    responseMessage.type = READERID;
    status = msgsnd(encryptID, &responseMessage, (sizeof(ThreadMessage) - sizeof(long)), 0);
    if(status < 0) {
        perror("thread noti queue send failed");
        exit(1);
    }

    while(1) {
        
        encryptMessage.type = currentInfo->type;
        if ((status = msgrcv(encryptID , &encryptMessage, (sizeof(ThreadMessage) - sizeof(long)), currentInfo->type,0)) < 0) {
            perror("Noti receive failed at the Work thread");
            exit(1);
        }
        if(encryptMessage.encryptStatus == IPC_ABORT)
            break;
        currentInfo->state = TINPROGRESS;
        XOROperation(encryptMessage.buf->data, encryptMessage.buf->len, encryptMessage.buf->key, encryptMessage.buf->len);
        free(encryptMessage.buf->key);
        responseMessage.type = READERID;
        responseMessage.id = currentInfo->type;
        currentInfo->state = TOK;
        encryptMessage.buf->status = OVERALL_DONE;
        msgsnd(encryptID, &responseMessage, (sizeof(ThreadMessage) - sizeof(long)), 0);
        responseMessage.type = WRITERID;
        msgsnd(encryptID, &responseMessage, (sizeof(ThreadMessage) - sizeof(long)), 0);
    }
    currentInfo->state = TABORT;
    return NULL;
}

void *WriteThreadWrite(void *ptr) {
    ThreadMessage responseMessage;
    inputData *currentInputData;
    node *newNode;
    responseMessage.type = WRITERID;
    
    while(1) {
        if(msgrcv(encryptID , &responseMessage, (sizeof(ThreadMessage) - sizeof(long)), WRITERID, 0) < 0) {
            perror("writer message receive failed");
            exit(1);
        }
        if(responseMessage.encryptStatus == IPC_ABORT)
            break;
        newNode = peak(writerQueue);
        if(newNode) {
            currentInputData = (inputData*)newNode->data;
            if(currentInputData->status == OVERALL_DONE) {
                newNode = dequeue(writerQueue);
                fwrite(currentInputData->data, currentInputData->len, 1, stdout);
                free(currentInputData->data);
                free(currentInputData);
                free(newNode);
            }
        }
    }
    while((newNode = dequeue(writerQueue)) != NULL) {
        currentInputData = (inputData*)newNode->data;
        fwrite(currentInputData->data, currentInputData->len, 1, stdout);
        free(currentInputData->data);
        free(currentInputData);
        free(newNode);
    }
    return NULL;
}

void freeAllThreads() {
    unsigned int i;
    ThreadMessage del_msg;
    del_msg.encryptStatus = IPC_ABORT;
    
    if (actualThreadNumber != 0) {
        for(i = 0; i < actualThreadNumber; i++) {
            del_msg.type = threadInfolist[i].type;
            if (msgsnd(encryptID , &del_msg, (sizeof(ThreadMessage) - sizeof(long)), 0) < 0) {
                perror("Noti Send to worker thread failed");
                exit(1);
            }
            pthread_join(threadInfolist[i].id, NULL);
        }
    }
    
    del_msg.type = WRITERID;
    if (msgsnd(encryptID , &del_msg, (sizeof(ThreadMessage) - sizeof(long)), 0) < 0) {
        perror("Noti Send to writer thread fail");
        exit(1);
    }
    pthread_join( writeThreadInfo.id, NULL);
    msgctl(encryptID, IPC_RMID, NULL);
}


void threadInit(unsigned int numberOfThreads) {
    key_t encode_msg_key;
    int msgflg = IPC_CREAT | IPC_EXCL | 0666;
    actualThreadNumber = 1;
    if (numberOfThreads > 0) {
        actualThreadNumber = numberOfThreads;
    }
    encode_msg_key = IPC_PRIVATE;
    
    if ((encryptID = msgget(encryptID, msgflg )) < 0) {
        perror("Message queue create failed:");
        exit(1);
    }

    writerQueue = new Queue();
    for(int i = 0; i < actualThreadNumber; i++) {
        threadInfolist[i].type = TBASEID + i;
        threadInfolist[i].state = TOK;
        if(pthread_create( &threadInfolist[i].id, NULL, WorkThreadWork, &threadInfolist[i]) != 0) {
            perror("Thread Create Failed");
            exit(1);
        }
    }
    if (pthread_create( &writeThreadInfo.id, NULL, WriteThreadWrite, &writeThreadInfo) != 0) {
        perror("Thread Create Failed");
        exit(1);
    }

}

void updateThreadupdate (unsigned char *section, unsigned long sectionLength, unsigned char *key) {
    ThreadMessage encryptMessage;
    ThreadMessage threadNoti;
    inputData *newInputData = NULL;
    long status;
    
    threadNoti.type = READERID;
    status = msgrcv(encryptID, &threadNoti, (sizeof(ThreadMessage) - sizeof(long)), READERID, 0);
    if(status < 0) {
        perror("Noti receive fail");
        exit(1);
    }
   
    newInputData = new inputData();
    if(newInputData == NULL) {
        perror("Memory allocation Failed");
        exit(1);
    }
    newInputData->len = sectionLength;
    newInputData->data = section;
    newInputData->key = key;
    newInputData->status = OVERALL_INPROGRESS;

    encryptMessage.type = threadNoti.id;
    encryptMessage.buf = newInputData;
    encryptMessage.encryptStatus = IPC_WORK;
    enqueue(writerQueue, newInputData);
    
    status = msgsnd(encryptID, &encryptMessage, (sizeof(ThreadMessage) - sizeof(long)), 0);
    if(status < 0) {
        perror("update noti send failed");
        exit(1);
    }
}
