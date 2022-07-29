#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<semaphore.h> 

typedef struct CpuData
{
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;
    unsigned long long guest;
    unsigned long long guest_nice;
}CpuData;

pthread_mutex_t cpuDataMutex;
sem_t semReadyToAnalyze, semReadyToPrint; 
CpuData *prevReaded = NULL, *nowReaded = NULL;
float CPU_Percentage = 0.0;
int divider = 1, readerCounter = 0, analyzerCounter = 0, printerCounter = 0;

void setDefaultValuesCpuData(CpuData *cpuData){
    cpuData = malloc(sizeof(CpuData));
    cpuData->guest = 0;
    cpuData->guest_nice = 0;
    cpuData->idle = 0;
    cpuData->iowait = 0;
    cpuData->irq = 0;
    cpuData->nice = 0;
    cpuData->softirq = 0;
    cpuData->steal = 0;
    cpuData->system = 0;
    cpuData->user = 0;
}

void getCpuTime(CpuData *cpuData) {
    FILE* file = fopen("/proc/stat", "r");
    if (file == NULL) {
        perror("There was a problem opening the file");
        return;
    }

    char buffer[1024];

    char* ret = fgets(buffer, sizeof(buffer) - 1, file);
    if (ret == NULL) {
        perror("There was a problem starting to read /proc/stat");
        fclose(file);
        return;
    }
    fclose(file);

    sscanf(buffer,
        "cpu  %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu",
        &(cpuData->user), &(cpuData->nice), &(cpuData->system), &(cpuData->idle),
        &(cpuData->iowait), &(cpuData->irq), &(cpuData->softirq), &(cpuData->steal),
        &(cpuData->guest), &(cpuData->guest_nice));

    return;
}

void *reader(){

    setDefaultValuesCpuData(prevReaded);
    setDefaultValuesCpuData(nowReaded);
    getCpuTime(nowReaded);
    usleep(20000);

    while (1)
    {
        pthread_mutex_lock(&cpuDataMutex);
        prevReaded = nowReaded;
        getCpuTime(nowReaded);
	    sem_post(&semReadyToAnalyze);
        pthread_mutex_unlock(&cpuDataMutex);
        usleep(20000);
        readerCounter++;
    }
    
    return NULL;
}

void *analyzer(){

    while (1)
    {
        sem_wait(&semReadyToAnalyze);
        pthread_mutex_lock(&cpuDataMutex);

        long long PrevIdle = (prevReaded->idle) + (prevReaded->iowait);
        long long Idle = (nowReaded->idle) + (nowReaded->iowait);
        long long PrevNonIdle = (prevReaded->user) + (prevReaded->nice) + (prevReaded->system) + (prevReaded->irq) 
                                + (prevReaded->softirq) + (prevReaded->steal);
        long long NonIdle = (nowReaded->user) + (nowReaded->nice) + (nowReaded->system) + (nowReaded->irq) 
                            + (nowReaded->softirq) + (nowReaded->steal);

        long long totald = (Idle + NonIdle) - (PrevIdle + PrevNonIdle);

        CPU_Percentage = (((totald - (Idle - PrevIdle))/totald) * 100 + CPU_Percentage) / divider;
        divider = 2;
        analyzerCounter++;
        pthread_mutex_unlock(&cpuDataMutex);
        
        sem_post(&semReadyToPrint);
    }

    return NULL;
}

void *printer(){
    
    while (1)
    {
        sem_wait(&semReadyToPrint);
        sleep(1);
        printf(" \n CPU Percentage = %f \n", CPU_Percentage);
        divider = 1;
        CPU_Percentage = 0.0;
        printerCounter++;
    }
    
    return NULL;
}

void *watchdog(){
    
    int readerCounterOld = readerCounter, analyzerCounterOld = analyzerCounter; 
    int printerCounterOld = printerCounter;

    while (1)
    {
        sleep(2);
        if (readerCounterOld == readerCounter || analyzerCounterOld == analyzerCounter
            || printerCounterOld == printerCounter)
        {
            printf("The threads do not respond in 2 sec. Exiting all process...");
            pthread_exit(reader);
            pthread_exit(analyzer);
            pthread_exit(printer);
            exit(2);   
        }
        if (readerCounter >= 20000)
            readerCounter = 0;

        if (analyzerCounter >= 20000)
            analyzerCounter = 0;

        if (printerCounter >= 20000)
            printerCounter = 0;
        
        readerCounterOld = readerCounter;
        analyzerCounterOld = analyzerCounter; 
        printerCounterOld = printerCounter;
    }

    return NULL;
}

int main(){
     pthread_t readerT, analyzerT, printerT, watchdogT;
     pthread_mutex_init(&cpuDataMutex, NULL);

    sem_init(&semReadyToAnalyze, 0, 0);
    sem_init(&semReadyToPrint, 0, 0);

    if (pthread_create(&readerT, NULL, &reader, NULL) != 0 ||
        pthread_create(&analyzerT, NULL, &analyzer, NULL) != 0 ||
        pthread_create(&printerT, NULL, &printer, NULL) != 0 ||
        pthread_create(&watchdogT, NULL, &watchdog, NULL) != 0){

        perror("There was an error while creating a threads \n");
        return 1;
    }
    if (pthread_join(readerT, NULL) != 0 || pthread_join(analyzerT, NULL) != 0 ||
        pthread_join(printerT, NULL) != 0 || pthread_join(watchdogT, NULL) != 0){
        
        perror("There was an error while pthread_join() in main");
        if (nowReaded != NULL)
            free(nowReaded);
        if (prevReaded != NULL)
            free(prevReaded);

        pthread_mutex_destroy(&cpuDataMutex);
        return 2;
    }
    sem_destroy(&semReadyToAnalyze);
    sem_destroy(&semReadyToPrint);

    return 0;
}