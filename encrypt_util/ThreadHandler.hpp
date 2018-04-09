#ifndef ThreadHandler_hpp
#define ThreadHandler_hpp

#include "MessageQueues.hpp"
#include <stdio.h>
#include <pthread.h>
#include <sys/ipc.h>
#include "stdlib.h"
#include <sys/msg.h>
#include <sys/types.h>

#define TBASEID 5000
#define TOK  1
#define TINPROGRESS   2
#define TABORT   -1
#define READERID 2000
#define WRITERID 3000
#define OVERALL_INPROGRESS 1
#define OVERALL_DONE 0
#define IPC_ABORT 2
#define IPC_WORK  4


class ThreadInfo {
public:
    pthread_t id;
    int type;
    char state;
};

class inputData {
public:
    unsigned char *data;
    unsigned char *key;
    unsigned long len;
    unsigned int status;
};

class ThreadMessage {
public:
    long type;
    unsigned int id;
    inputData *buf;
    unsigned char encryptStatus;
};

void threadInit(unsigned int numberOfThreads);
void freeAllThreads();
void updateThreadupdate (unsigned char *section, unsigned long sectionLength, unsigned char *key);
#endif /* ThreadHandler_hpp */
