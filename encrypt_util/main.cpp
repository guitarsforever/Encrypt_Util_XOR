
#include <iostream>
#include "ThreadHandler.hpp"
#define MAX_KEY_SIZE 1024
#define MAX_NUM_THREAD 50
#define MINIMUM_LENGTH 3
//unsigned char ** keyDump; *deprecated*

void rotateKey (unsigned char * key, unsigned long keyLength) {
    
    long i, carryBack= 0;
    if(key[keyLength-1] & (1<<(CHAR_BIT - 1))) {
        carryBack = 1;
    }
    for(i= keyLength-1; i>=0; i--){
        unsigned char carry = 0;
        if(i!= 0 && (key[i-1] & (1<<(CHAR_BIT - 1)))){
            carry = 1;
        }
        key[i] <<= 1;
        key[i] |= carry;
    }
    if(carryBack == 1){
        key[0] |= 0x01;
    }
}

void keyGen(unsigned char *retreivedKey, unsigned long len, unsigned char *key, unsigned long keyLength) {
    unsigned char *localKey = retreivedKey;
    unsigned long count = 0;
    unsigned long incrementer = 0;
    
    while(count < len) {
        if(len - count < keyLength)
            incrementer = len - count;
        else
            incrementer = keyLength;
        memcpy(localKey, key, incrementer);
        count += incrementer;
        localKey += incrementer;
        rotateKey(key, keyLength);
    }
    return;
}

void processInput (unsigned char * key, unsigned long keyLength) {
    unsigned char *retreivedKey;
    unsigned long len;
    unsigned char * buffer;
    unsigned long readerSize = keyLength;
    unsigned long i = 0;
    
    if (readerSize < MAX_KEY_SIZE) {
        readerSize = (MAX_KEY_SIZE/keyLength) * keyLength;
    }
    
    do {
    
        if (i == keyLength) i = 0;
        buffer = new unsigned char [readerSize];
        if(buffer == NULL) {
            std::cerr <<  "buffer allocation failure" << std::endl;
            return;
        }
        
        len = fread(buffer, 1, readerSize, stdin);
        if(len) {
            retreivedKey = new unsigned char[len];
            if(retreivedKey == NULL) {
                std::cerr <<  "buffer allocation failure" << std::endl;
                return;
            }
            keyGen(retreivedKey, len, key, keyLength);
            //retreivedKey = keyDump[i]; *deprecated*
            updateThreadupdate(buffer, len,retreivedKey);
        }else {
            free(buffer);
        }
        i++;
    } while(len);
    std::cerr << "completed" << std::endl;
}

/*  *deprecated*
void generateAllKeys( unsigned char * key,unsigned long  keyLength) {
    keyDump = new unsigned char * [CHAR_BIT];
    if(keyDump == NULL)
    {
        std::cerr << std::endl << "out of mem"<< std::endl;
        return;
    }
    
    for(int i = 0; i <keyLength * CHAR_BIT; i++)
    {
        keyDump[i] =  new unsigned char [keyLength];
        if(keyDump[i] == NULL)
        {
            std::cerr << std::endl << "out of mem"<< std::endl;
            return;
        }
    }
    
    int j=0;
    do
    {
        memcpy(keyDump[j],key,keyLength);
        std::cerr << "In generateallkeys: " << std::hex <<keyDump[j]<< std::endl;
        rotateKey(key,keyLength);
        j++;
        
    }while(j < (keyLength * CHAR_BIT) );
} */

long getFileSize(FILE * filePointer)
{
    long size = 0;
    fseek(filePointer, 0L, SEEK_END);
    size = ftell(filePointer);
    fseek(filePointer, 0L, SEEK_SET);
    return size;
}

void error() {
    
    std::cerr << "encryptUtil [-n #] [-k keyfile]" << std::endl <<"-n # Number of threads to create(<50 threads)" << std::endl <<"-k keyfile Path to file containing key" <<std::endl ;
}


int main(int argc, const char * argv[]) {
    unsigned char key[MAX_KEY_SIZE];
    unsigned long keyLength = 0;
    int numberOfThreads = 0;
    char *keyFileName = NULL;
    FILE *fp = NULL;
    
    if(argc < MINIMUM_LENGTH) {
        error();
        return 1;
    }
    
    for (int i = 1; i < (argc - 1); i++) {
        if (strcmp("-n", argv[i]) == 0) {
            numberOfThreads = atoi(argv[++i]);
            if (numberOfThreads == 0 || numberOfThreads >= MAX_NUM_THREAD) {
                error();
                return 1;
            }
            continue;
        }
        if (strcmp("-k", argv[i]) == 0) {
            keyFileName = (char *)argv[++i];
            continue;
        }
         error();
        return 1;
    }
    if (keyFileName == NULL) {
        error();
        return 1;
    }
    fp=fopen(keyFileName,"rb");
    if (!fp){
        std::cerr<<std::endl<<"Unable to open key file!"<<std::endl;
        return 1;
    }
    
    keyLength  = fread(key, 1, MAX_KEY_SIZE, fp);
    if (keyLength == MAX_KEY_SIZE) {
        std::cerr<< "Key Length too big: Provide a key file of size less than " << MAX_KEY_SIZE <<std::endl;
        fclose(fp);
        return 1;
    }
    
    if(keyLength == 0) {
        std::cerr<< "Empty key File " <<std::endl;
        fclose(fp);
        return 1;
    }
    
    
    fclose(fp);
    //generateAllKeys (key,keyLength);
    threadInit(numberOfThreads);
    processInput(key,keyLength);
    freeAllThreads();
    
    return 0;
}
